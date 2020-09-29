#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

class Client
{
private:

public:
    int clientFD;                               //FD to communicate witht he client
    struct sockaddr_storage clientAddress;      //struct to store client network information
    socklen_t clientLen;                        //length of client addrinfo struct
    int status;                                 //status of client
    void initClient();
};


#endif