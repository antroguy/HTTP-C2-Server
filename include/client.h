#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <string>
#include <netdb.h>
#include <poll.h>

class Client
{
private:

public:
    int clientFD;                               //FD to communicate witht he client
    struct sockaddr_storage clientAddress;      //struct to store client network information
    struct pollfd pfds[1];
    socklen_t clientLen;                        //length of client addrinfo struct
    std::string status;                                 //status of client
    std::string method;                         //method of request  
    std::string path;                           //request path (File request or File upload)
    std::string version;                        //HTTP Version  
    char host[NI_MAXHOST];
    char port[NI_MAXSERV];                  
    void initClient();
};


#endif