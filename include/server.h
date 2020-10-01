#ifndef SERVER_H
#define SERVER_H

#include <netdb.h>
#include <errno.h>
#include <string>
#include <string.h>
#include "client.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>

#define BUFFER 4096

class Server{
private:
    unsigned int maxpending;                //Maximum pending connections that can be qued up before connections are refused
    struct addrinfo server;                 //Addrinfo struct for server network parameters 
    struct addrinfo *serverInfo;            //Used to point to results
    int option_value;                       //Option value will be set to 1 to set REUSEADDR for SO_Socket

public:
    Server(unsigned int maxpending, std::string port, std::string ip);       //Constructor
    int serverFD;                                                            //Server file Descriptor
    //public methods
    int initServer(unsigned int maxpending, std::string port, std::string ip);  //Initialize values for addrinfo
    int parseCommand(Client *Client);            //Parse data
    size_t serverSendResponse(Client *Client);  //Send Response
    size_t serverSendHeader(Client *Client);   //Send Header
    int cleanup(Client *Client);    //Cleanup Client Context
    void perform();

    
    
};

#endif