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
        clientContext->clientFD = accept(this->serverFD,(struct sockaddr *)&clientContext->clientAddress,&clientContext->clientLen);

        if(clientContext->clientFD == -1){
            std::exit(EXIT_FAILURE);
        }

        char buffer[1024] = {0};
        int valread = read(clientContext->clientFD,buffer,1024);
        printf("%s\n",buffer);
        write(clientContext->clientFD, "Hellow",8);
    }
}