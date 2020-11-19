#ifndef SERVER_H
#include "server.h"
#endif
//Constructor
Server::Server(unsigned int c_maxpending, std::string port){
    initServer(maxpending,port);
}
//Initialize Server Variables
int Server::initServer(unsigned int maxpending, std::string port){
    this->maxpending = maxpending;
    this->port = port;
    //Set server,serverInfo  memory to 0, getaddrinfo() will fail without this.
    memset(&this->server,0,sizeof(addrinfo));
    //this->serverInfo = NULL;
    this->option_value = 1;
    //Assign the socket family (IPV4, socktype (TCP = Socks_tream), and flags (AI_PASSIVE - Wildcard INADDR_ANY)
    this->server.ai_family = AF_INET;
    this->server.ai_socktype = SOCK_STREAM;
    //If ai_flags is set to AI_PASSIVE and node (getaddr arg 1) is NULL, returned socket address will be INADDR_ANY to accept connections on any of the host's network addresses.
    this->server.ai_flags = AI_PASSIVE; 
 

}
//recieve and validate header for method
Server::Status Server::recvRequest(Client *Client){
        //Buffer to hold information
        char buffer[BUFFER] = {0}; 
        //Map to store header attributes/values         
        std::map<std::string,std::string> headermap; //Allocate to heap instead of stack? 
        //Setup polling to detect incoming data
        Client->pfds[0].fd = Client->clientFD;
        Client->pfds[0].events = POLLIN; //There is data to read
        int pollVal = 0;
        //Read request from client
        while((pollVal = poll(Client->pfds,1,100)) > 0){
            DEBUG_PRINT((stdout,"Recieving Request\n"));
            if(recv(Client->pfds[0].fd,buffer,BUFFER, 0) == -1){
                DEBUG_PRINT((stdout,"Failed to read from client: Errno: %s\n", strerror(errno)));
                return Status::STATUS_ERROR;
            }
        }
        //Validate buffer isnt null, if so ignore request
        if(strlen(buffer) ==0){
            return Status::STATUS_ERROR;
        }
        //Parse the header for Method, Path, and HTTP Version
        if(parseHeader(buffer,&headermap) == Status::STATUS_INVALID_REQUEST){
            DEBUG_PRINT((stdout,"Invalid Header: \"%s\" from client: %s\n",headermap.at("Method").c_str(), Client->host));
            Client->status = statResp.Status_400;
            return Status::STATUS_INVALID_REQUEST;
        }     
        DEBUG_PRINT((stdout,"Successfully read request header\n"));
        //Assign values to header.
        Client->method = headermap.at("Method");
        Client->path = headermap.at("Path").substr(headermap.at("Path").find("/") + 1,headermap.at("Path").size());
        //If Post request, need to process additional data upload
        if(Client->method == "POST"){
            //Set seed for new number
            srand(time(NULL));
            //Create random file for processing
            std::string dir = Client->path + "/" + std::to_string(rand()) + ".png";
            //Lock
            mapMutex.lock();
            //Open file for writing
            FILE *file = fopen(dir.c_str(), "wb");
            //Update Map
          
            if(file == nullptr){
                return Status::STATUS_ERROR;
            }
            fileMap.insert(std::make_pair(dir,1));
            mapMutex.unlock();
            //Temp Buffer 
            char *recvBuff = (char *)malloc(sizeof(char) * BUFFER);
            //Keep track of bytes recieved
            int bytesRecieved = 0;
            int result = 0;
            while(bytesRecieved < atoi(headermap.at("Content-Length").c_str())){
                if((result = recv(Client->clientFD,recvBuff,BUFFER,0)) == -1 ){
                        break;
                }
                fwrite(recvBuff,1,result,file);
                bytesRecieved += result;
            }
            if(bytesRecieved != atoi(headermap.at("Content-Length").c_str())){
                fclose(file);
                free(recvBuff);
                remove(dir.c_str());
                return Status::STATUS_ERROR;
            }
            fclose(file);
            free(recvBuff);
            mapMutex.lock();
            fileMap.find(dir)->second = 0;
            mapMutex.unlock();
            fileCond.notify_all();


        }
        //Check if default Path
        if(Client->path ==DEFAULT_PATH){
            fprintf(stdout,"Client %s has initiated connection\n", Client->host);
        }
        DEBUG_PRINT((stdout,"Request method: %s %s from client: %s\n",headermap.at("Method").c_str(),headermap.at("Path").c_str(), Client->host));
        Client->status = statResp.Status_200;
        return Status::STATUS_OK;
}
//Pare header for attributes and values
Server::Status Server::parseHeader(char *buf, std::map<std::string,std::string> *headerMap){
    std::istringstream resp(buf);       //Input stream for header
    std::string output;                 //Temmpory string to hold each line of the ehader request
    int index,prev = 0;                 //Indexs
    int countInitial =0;                //Used to parse Method, Path, and HTTP Version
    //Extract characters fromt he stream (Delim character is \n by default)
    while(std::getline(resp,output) && output != "\r"){
        //If initial, grab Method, Path and HTTP Version
        if(countInitial == 0){
            index = output.find(' ', 0);
            while(index != std::string::npos){
                headerMap->insert(std::make_pair(headerFormat[countInitial], output.substr(prev,index-prev)));
                prev = index + 1;
                index = output.find(" ", prev);
                countInitial++;
            }
        //Else grab all other attributes and values
        }else{
            index = output.find(':', 0);
            if(index != std::string::npos){
                headerMap->insert(std::make_pair(output.substr(0,index),output.substr(index+2, output.length() - index -3)));
            }
        }
    }
    //Validate header is valid
    try{
        //Check if empty
        if(headerMap->empty()){
            return Status::STATUS_INVALID_REQUEST;
        }   
        //Check for valid method
        if(headerMap->at("Method") != "GET" && headerMap->at("Method") !="POST"){
            return Status::STATUS_INVALID_REQUEST;
        }
        if(headerMap->at("Method") == "POST"){
            if(headerMap->at("Content-Length").empty()){
              return Status::STATUS_INVALID_REQUEST;
            }
        }
        //Check if path is empty
        if(headerMap->at("Path") == ""){
            return Status::STATUS_INVALID_REQUEST;
        }
    }catch (const std::out_of_range error){
        DEBUG_PRINT((stdout,"Invalid header request\n"));
        return Status::STATUS_INVALID_REQUEST;
    }
    return Status::STATUS_OK;
}
//send header
Server::Status Server::serverSendHeader(Client *Client){
    //FD for image
    int fileFD;
    off_t fileLength = 0;
    std::string message;
    //If invalid header recieved, send back response
    if(Client->status == statResp.Status_400){
        Client->status = statResp.Status_400;
        Client->contenType = "text/plain";
        fileLength = sizeof(Client->status.c_str());
    }
    else if(Client->method == "GET"){
        if((fileFD = open(Client->path.c_str(),O_RDONLY)) == -1){
            DEBUG_PRINT((stderr,"File Not Found: file: %s from client: %s\n",Client->path.c_str(), Client->host));
            close(fileFD);
            //set request status
            Client->status = statResp.Status_404;
            Client->contenType = "text/plain";
            fileLength = sizeof(Client->status.c_str());
        }else{
            Client->contenType = "image/png";
            //structure to store file statistics
            struct stat fileStats;
            //get the length of the file using fstats://NOTE FSTAT IS NOT THREAD SAFE - CHANGE LATER
            fstat(fileFD, &fileStats);
            fileLength = fileStats.st_size;
            //close FD
            close(fileFD);
            //Set request status
            Client->status = statResp.Status_200;
        }
        message = Client->version + Client->status + "\r\nContent-Type: " + Client->contenType + "\r\nContent-Length: " + std::to_string(fileLength) + "\r\n\r\n";
        //std::string test = "HTTP/1.1 200 OK\nContent-Type: image/jpeg\nContent-Length: 146782\n\n";
        if(send(Client->clientFD, message.c_str(),message.length(),0) == -1){
            DEBUG_PRINT((stdout,"%Error sending header: Errno - %s\n",strerror(errno)));
            return Status::STATUS_ERROR;
        }
    //Will Implement this Feature Later
    }else if(Client->method == "POST"){
        Client->status = statResp.Status_201;
        message = Client->version + Client->status + "\r\n\r\n";
        //std::string test = "HTTP/1.1 200 OK\nContent-Type: image/jpeg\nContent-Length: 146782\n\n";
        if(send(Client->clientFD, message.c_str(),message.length(),0) == -1){
            DEBUG_PRINT((stdout,"%Error sending header: Errno - %s\n",strerror(errno)));
            return Status::STATUS_ERROR;
        }
    }

        //Send header
    DEBUG_PRINT((stdout,"%s\n",message.c_str()));
    return Status::STATUS_OK;

}
//Send data back to client
Server::Status Server::serverSendBody(Client *Client){
    if(Client->status == statResp.Status_404 || Client->status == statResp.Status_400){
        if(send(Client->clientFD,Client->status.c_str(),sizeof(Client->status.c_str()),0) == -1){
            DEBUG_PRINT((stdout,"%Error sending body: Errno - %s\n",strerror(errno)));
            return Status::STATUS_ERROR;
        }
        return Status::STATUS_OK;
    }
    else if(Client->method == "GET"){   
        int fileFD = open(Client->path.c_str(),O_RDONLY); 
        off_t offset = 0;
        //Create a temporary buffer to store data
        char *bufferW = (char*) malloc(sizeof(char)*BUFFER);
        //Set the initial address for the  buffer so it can be reset if needed
        char *bufferOffset = bufferW;
        size_t bytesRecieved = 0;
        int bytesSent = 0;
        //Read from the file and store it in buffer.
        while((bytesRecieved = pread(fileFD,bufferW,4096,offset)) > 0){
            //Increment byte offset of the file
            offset += bytesRecieved;
            //Send data to client
            while((bytesSent = send(Client->clientFD,bufferW,bytesRecieved,0)) != bytesRecieved){
                bufferOffset +=bytesSent;
                bytesRecieved = bytesRecieved-bytesSent;
                if(bytesSent == -1){
                    DEBUG_PRINT((stdout,"Failed to send file: Errno - %s\n", strerror(errno)));
                    free(bufferW);
                    bufferW = NULL;
                    close(Client->clientFD);
                    return Status::STATUS_ERROR;
                }
            }
            //cleanup the buffer
            memset(bufferW,0,sizeof(bufferW));
        }
        if(bytesRecieved == -1){
            DEBUG_PRINT((stdout,"Pread failed to read from file: Errno - %s\n", strerror(errno)));
            free(bufferW);
            bufferW = NULL;
            close(Client->clientFD);
            return Status::STATUS_ERROR;
        }
        //fprintf(stdout,"%s\n",command1.c_str());
        //send(Client->clientFD,this->command1.c_str(),this->command1.length(),0);
        DEBUG_PRINT((stdout,"Body Sent to client successfully: %s\n", Client->host));
        //Free the buffer
        free(bufferW);
        bufferW = NULL;
        close(Client->clientFD);
        return Status::STATUS_OK;
    }
    else if(Client->status == "POST"){
        return Status::STATUS_OK;
    }
    return Status::STATUS_ERROR;
}
//Cleanup (Need to implement more later)
void Server::cleanup(Client *Client){
    if(Client->clientFD != NULL){
        close(Client->clientFD);
    }
    delete(Client);
}
//Perform certain functions
void Server::perform(){
    int status;
    if((status = getaddrinfo(NULL,this->port.c_str(),&this->server,&this->serverInfo)) != 0){
        DEBUG_PRINT((stdout,"Failed to get addr info\n"));
        std::exit(EXIT_FAILURE);
    }
    //Create the server socket and save the FD
    if((this->serverFD = socket(this->serverInfo->ai_family,this->serverInfo->ai_socktype,this->serverInfo->ai_protocol)) == -1){
        DEBUG_PRINT((stdout,"Failed to create socket\n"));
        std::exit(EXIT_FAILURE);
    }
    //Set sockopt SO_REUSEADDR to avoid bind errors if the address is currently in use
    if(setsockopt(this->serverFD,SOL_SOCKET,SO_REUSEADDR,&this->option_value, sizeof(this->option_value)) == -1){
        DEBUG_PRINT((stdout,"Failed to setopt for socket: \n"));
        std::exit(EXIT_FAILURE);
    }
    //Bind to the socket
    if(bind(this->serverFD,this->serverInfo->ai_addr,this->serverInfo->ai_addrlen) != 0){
        DEBUG_PRINT((stdout,"Failed to bind to socket: Errno %s\n",strerror(errno)));
        std::exit(EXIT_FAILURE);
    }

    //listen for incoming connections
    if(listen(this->serverFD, this->maxpending) != 0){
        DEBUG_PRINT((stdout,"Failed to listen for connections: Errno - %s\n", strerror(errno)));
        std::exit(EXIT_FAILURE);
    }
    //free the addrinfo ptr, no longer needed
    freeaddrinfo(this->serverInfo);
    fprintf(stdout,"Server Initialized.........................\n");
    while(true){
        //Create client context object
        Client *clientContext = new Client();
        clientContext->initClient();
        //Create ClientContext
        clientContext->clientLen = sizeof(sockaddr_storage);
        //Accept client connection
        clientContext->clientFD = accept(this->serverFD,(struct sockaddr *)&clientContext->clientAddress,&clientContext->clientLen);
        //Check if connection was successfully established
        if(clientContext->clientFD == -1){
            //Cleanup
            //Continue
            continue;
        }
        //Get Client Address
        if(getnameinfo((struct sockaddr *)&clientContext->clientAddress,sizeof(sockaddr_storage),clientContext->host,sizeof(clientContext->host),clientContext->port,sizeof(clientContext->port),NI_NUMERICHOST | NI_NUMERICSERV) != 0){
            DEBUG_PRINT((stdout,"Failed to get Client Address\n"));
            
        }
        //Parse header for command
        if((recvRequest(clientContext)) == Status::STATUS_ERROR){
            cleanup(clientContext);
            continue;
        }
        //Generate Request
        if(serverSendHeader(clientContext) == Status::STATUS_ERROR){
            cleanup(clientContext);
            continue;
        }
        //Send Response
        if(serverSendBody(clientContext) == Status::STATUS_ERROR){
            cleanup(clientContext);
            continue;
        }
    
        cleanup(clientContext);
    }


}

Server::Status Server::serverHandler(){
    int status;
    if((status = getaddrinfo(NULL,this->port.c_str(),&this->server,&this->serverInfo)) != 0){
        DEBUG_PRINT((stdout,"Failed to get addr info\n"));
        return Status::STATUS_ERROR;
    }
    //Create the server socket and save the FD
    if((this->serverFD = socket(this->serverInfo->ai_family,this->serverInfo->ai_socktype,this->serverInfo->ai_protocol)) == -1){
        DEBUG_PRINT((stdout,"Failed to create socket\n"));
        return Status::STATUS_ERROR;
    }
    //Set sockopt SO_REUSEADDR to avoid bind errors if the address is currently in use
    if(setsockopt(this->serverFD,SOL_SOCKET,SO_REUSEADDR,&this->option_value, sizeof(this->option_value)) == -1){
        DEBUG_PRINT((stdout,"Failed to setopt for socket: \n"));
        close(this->serverFD);
        return Status::STATUS_ERROR;
    }
    //Bind to the socket
    if(bind(this->serverFD,this->serverInfo->ai_addr,this->serverInfo->ai_addrlen) != 0){
        DEBUG_PRINT((stdout,"Failed to bind to socket: Errno %s\n",strerror(errno)));
        close(this->serverFD);
        return Status::STATUS_ERROR;
    }

    //listen for incoming connections
    if(listen(this->serverFD, 3) != 0){
        DEBUG_PRINT((stdout,"Failed to listen for connections: Errno - %s\n", strerror(errno)));
        close(this->serverFD);
        return Status::STATUS_ERROR;
    }
    fprintf(stdout, "Handler listening for incoming connection on port %s\n",this->port.c_str());
    //free the addrinfo ptr, no longer needed
    freeaddrinfo(this->serverInfo);
     //Create client context object
    Client *clientContext = new Client();
    clientContext->initClient();
    //Create ClientContext
    clientContext->clientLen = sizeof(sockaddr_storage);
    //Accept client connection
    //Setup polling  to listen for events on the listening fd (Server)
    clientContext->pfds[0].fd = this->serverFD;
    clientContext->pfds[0].events = POLLIN; //There is data to read
    int pollVal = 0;
    //Poll on accept
    pollVal = poll(clientContext->pfds,1,200000);
  
    if(pollVal <1 ){
        DEBUG_PRINT((stdout,"Failed to accept incoming connection: Errno: %s\n", strerror(errno)));
        cleanup(clientContext);
        close(this->serverFD);
        return Status::STATUS_ERROR;
    }
    if((clientContext->clientFD = accept(this->serverFD,(struct sockaddr *)&clientContext->clientAddress,&clientContext->clientLen)) == -1){
       DEBUG_PRINT((stdout,"Failed to accept incoming connection: Errno: %s\n", strerror(errno)));
        cleanup(clientContext);
        close(this->serverFD);
       return Status::STATUS_ERROR;
    }
    std::string command;
    std::string endState;
    //Set pfds now to client fd to poll for data
    clientContext->pfds[0].fd = clientContext->clientFD;
    clientContext->pfds[0].events = POLLIN; //There is data to read
    //Pull for Input
    char *output = (char*)malloc(sizeof(char) * BUFFER); 
    memset(output,0,BUFFER);
    while(poll(clientContext->pfds,1, 1000) > 0){
        if(recv(clientContext->pfds[0].fd,output,BUFFER, 0) == -1){
            DEBUG_PRINT((stdout,"Failed to read from client: Errno: %s\n", strerror(errno)));
            cleanup(clientContext);
            free(output);
            close(serverFD);
            return Status::STATUS_ERROR;
        }
        fprintf(stdout,output,strlen(output));
        memset(output,0,BUFFER);
          
    }

 
    std::cout << output;
    memset(output,0,BUFFER);
    while(true){

        std::getline(std::cin,command);
        if(command == "exit"){
            free(output);
            cleanup(clientContext);
            close(serverFD);
            return Status::STATUS_OK;
        }
        command.append("\r\n");
        if(send(clientContext->clientFD,command.c_str(),command.size(),0) == -1){
            DEBUG_PRINT((stdout,"%Error sending body: Errno - %s\n",strerror(errno)));
            free(output);
            cleanup(clientContext);
            close(serverFD);
            return Status::STATUS_ERROR;
        }

        while((pollVal = poll(clientContext->pfds,1, 10000)) > 0){
            if(recv(clientContext->pfds[0].fd,output,BUFFER, 0) == -1){
                DEBUG_PRINT((stdout,"Failed to read from client: Errno: %s\n", strerror(errno)));
                free(output);
                cleanup(clientContext);
                close(serverFD);
                return Status::STATUS_ERROR;
            }
            endState.append(output,strlen(output));
            auto endPos = endState.find("END$$");
            if(endPos != std::string::npos){
                endState.erase(endPos,5);
                fprintf(stdout,endState.c_str(),strlen(output));
                memset(output,0,BUFFER);
                endState.clear();
                break;
            }
            endState.clear();
            fprintf(stdout,output,strlen(output));
            memset(output,0,BUFFER);

          
        }
        fflush(stdout);
  
        command.clear();

    }
}