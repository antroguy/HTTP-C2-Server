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
    fprintf(stdout,"Server Initialized.........................\n");

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
        //Validate header

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
        if(headerMap->at("Method") != "GET" && "POST"){
            return Status::STATUS_INVALID_REQUEST;
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
    //Will Implement this Feature Later
    }else if(Client->method == "PUT"){
        //DoSomthing
    }
    std::string message = Client->version + Client->status + "\r\nContent-Type: " + Client->contenType + "\r\nContent-Length: " + std::to_string(fileLength) + "\r\n\r\n";
    //std::string test = "HTTP/1.1 200 OK\nContent-Type: image/jpeg\nContent-Length: 146782\n\n";
    if(send(Client->clientFD, message.c_str(),message.length(),0) == -1){
        DEBUG_PRINT((stdout,"%Error sending header: Errno - %s\n",strerror(errno)));
        return Status::STATUS_ERROR;
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
    else if(Client->status == "PUT"){
        return Status::STATUS_OK;
    }
    return Status::STATUS_ERROR;
}
//Cleanup (Need to implement more later)
void Server::cleanup(Client *Client){
    close(Client->clientFD);
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