#ifndef SERVER_H
#include "server.h"
#endif
//Constructor
Server::Server(unsigned int c_maxpending, std::string port, std::string ip){
    initServer(maxpending,port,ip );
}
//Initialize Server Variables
int  Server::initServer(unsigned int maxpending, std::string port, std::string ip){
    this->maxpending = maxpending;
    //Set server,serverInfo  memory to 0, getaddrinfo() will fail without this.
    memset(&this->server,0,sizeof(addrinfo));
    //this->serverInfo = NULL;
    this->option_value = 1;
    //Assign the socket family (IPV4, socktype (TCP = Socks_tream), and flags (AI_PASSIVE - Wildcard INADDR_ANY)
    this->server.ai_family = AF_INET;
    this->server.ai_socktype = SOCK_STREAM;
    //If ai_flags is set to AI_PASSIVE and node (getaddr arg 1) is NULL, returned socket address will be INADDR_ANY to accept connections on any of the host's network addresses.
    this->server.ai_flags = AI_PASSIVE; 
    fprintf(stdout, "Server Initialized.........................\n");

}
//parse header for method
int Server::parseCommand(Client *Client){
        char buffer[BUFFER] = {0};
        //Read request from client
        if(recv(Client->clientFD,buffer,BUFFER, 0) == -1){
            fprintf(stdout,"Failed to read from client: Errno: %s\n", strerror(errno));
            //CLEANUP
        }
        if(getnameinfo((struct sockaddr *)&Client->clientAddress,sizeof(sockaddr_storage),Client->host,sizeof(Client->host),Client->port,sizeof(Client->port),NI_NUMERICHOST | NI_NUMERICSERV) != 0){
            fprintf(stdout,"Failed to get Client Address");
        }
        //Parse the header for Method, Path, and HTTP Version
        //Convert buffer to a string
        fprintf(stdout,"Successfully read request header");
        std::string header = buffer;
        //beginning of iterator
        int beginning = 0;
        //token to store substrings in
        std::vector<std::string> token;
        //Separate header by5 spaceing
        for(int end = 0; (end=header.find(" ",end)); ++end){
            token.push_back(header.substr(beginning,end - beginning));
            beginning = end +  1;
            //Only grabe the first three headers
            if(token.size() == 3){
                break;
            }
        }
        //Implement this later to ignore favicon.ico https://stackoverflow.com/questions/1321878/how-to-prevent-favicon-ico-requests
        if(token[1] == "/favicon.ico"){
            return -1;
        }
        //Check status, perform necessary options
        if(token[0] == "GET"){
            token[1].erase(token[1].find_first_of('/'),1);
            Client->method = "GET";
            Client->path = token[1];
            Client->version = token[2];
            fprintf(stdout,"Request method: %s %s from client: %s\n",token[0].c_str(),token[1].c_str(), Client->host);
            return 0;
        }else if(token[0] == "POST"){
            Client->method = "POST";
            Client->path = token[1];
            Client->version = token[2];
            fprintf(stdout,"Request method: %s %s from client: %s\n",token[0].c_str(),token[1].c_str(), Client->host);
            return 0;
        }else{
            fprintf(stdout,"Invalid request method: \"%s\" from client: %s\n",token[0].c_str(), Client->host);
            Client->status = "400 Bad Request";
            return -1;
        }

}
//send header
size_t Server::serverSendHeader(Client *Client){
    //FD for image
    int fileFD;
    off_t fileLength = 0;

    if((fileFD = open(Client->path.c_str(),O_RDONLY)) == -1){
        fprintf(stdout,"File Not Found: file: %s from client: %s\n",Client->path.c_str(), Client->host);
        close(fileFD);
        //set request status
        Client->status = "404 Not Found";
    }else{
        //structure to store file statistics
        struct stat fileStats;
        //get the length of the file using fstats://NOTE FSTAT IS NOT THREAD SAFE - CHANGE LATER
        fstat(fileFD, &fileStats);
        fileLength = fileStats.st_size;
        //close FD
        close(fileFD);
        //Set request status
        Client->status = "200 OK";
    }
    //Send header
    std::string message = Client->version + Client->status + "\nContent-Type: image/jpeg\nContent-Length: " + std::to_string(fileLength) + "\r\n\r\n";
    send(Client->clientFD, message.c_str(),message.length(),0);
}
//Send data back to client
size_t Server::serverSendResponse(Client *Client){
        int fileFD = open(Client->path.c_str(),O_RDONLY); 
        off_t offset = 0;
        //Create a temporary buffer to store data
        char *bufferW = (char*) malloc(4096);
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
            }
            //cleanup the buffer
            memset(bufferW,0,sizeof(bufferW));
        }
        //Free the buffer
        free(bufferW);
        bufferW = NULL;
        close(Client->clientFD);
}
//Perform certain functions
void Server::perform(){
    int status;
    if((status = getaddrinfo(NULL,"8080",&this->server,&this->serverInfo)) != 0){
        printf("Failed to get addr info");
        std::exit(EXIT_FAILURE);
    }
    //Create the server socket and save the FD
    if((this->serverFD = socket(this->serverInfo->ai_family,this->serverInfo->ai_socktype,this->serverInfo->ai_protocol)) == -1){
        printf("Failed to create socket");
        std::exit(EXIT_FAILURE);
    }
    //Set sockopt SO_REUSEADDR to avoid bind errors if the address is currently in use
    if(setsockopt(this->serverFD,SOL_SOCKET,SO_REUSEADDR,&this->option_value, sizeof(this->option_value)) == -1){
        printf("Failed to setopt for socket");
        std::exit(EXIT_FAILURE);
    }
    //Bind to the socket
    if(bind(this->serverFD,this->serverInfo->ai_addr,this->serverInfo->ai_addrlen) != 0){
        std::exit(EXIT_FAILURE);
    }

    //listen for incoming connections
    if(listen(this->serverFD, this->maxpending) != 0){
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
        //Parse header for command
        if((parseCommand(clientContext)) == -1){
            serverSendHeader(clientContext);
            close(clientContext->clientFD);
            continue;
        }
        //Check if file exists
        
        //Send Header
        serverSendHeader(clientContext);
        //Send Tree Image
        serverSendResponse(clientContext);
        close(clientContext->clientFD);

    }


}