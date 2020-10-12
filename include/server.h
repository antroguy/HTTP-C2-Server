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
#include <map>
#include <sstream>

#define BUFFER 4096

class Server{
private:
    unsigned int maxpending;                //Maximum pending connections that can be qued up before connections are refused
    struct addrinfo server;                 //Addrinfo struct for server network parameters 
    struct addrinfo *serverInfo;            //Used to point to results
    int option_value;                       //Option value will be set to 1 to set REUSEADDR for SO_Socket

public:
    std::string port;
    Server(unsigned int maxpending, std::string port);       //Constructor
    int serverFD;                                                            //Server file Descriptor
    //public methods
    int initServer(unsigned int maxpending, std::string port);  //Initialize values for addrinfo
    int recvRequest(Client *Client);                                           //Recieve Header Request
    int parseHeader(char * buf, std::map<std::string,std::string> *headerMap);  //parseHeader
    int serverSendBody(Client *Client);                                         //Send Response Body
    int serverSendHeader(Client *Client);                                        //Send Header
    int cleanup(Client *Client);                                                 //Cleanup Client Context
    void perform();

    
    
};

#endif