#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(x) fprintf x
#else
#define DEBUG_PRINT(X) do{} while(0)
#endif

#ifndef SERVER_H
#define SERVER_H

#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string>
#include <string.h>
#include "client.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <map>
#include "stegImage.h"
#include <sstream>
#include <mutex>
#include <condition_variable>


#define DEFAULT_PATH "images/default.png" //Default image client will read from
#define DEFAULT_RPATH "images/defaultR.png"   //Default image server will use to encode image from
#define DEFAULT_COMMAND "<COM:ALL:1-beacon-5000:5-0:1-path-/images/tree.png>"   //Default Command to execute (Initialization Sequence)
#define BUFFER 4096   

struct statusResponse{
        std::string Status_400 = "400 Bad Request";
        std::string Status_404 = "404 Not Found";
        std::string Status_200 = "200 OK";
        std::string Status_201 = "201 Created";
};
extern std::map<std::string,int> fileMap; 
//Mutex for files
extern std::mutex mapMutex;
//Condition variable
extern std::condition_variable fileCond;

class Server{
private:
    unsigned int maxpending;                //Maximum pending connections that can be qued up before connections are refused
    struct addrinfo server;                 //Addrinfo struct for server network parameters 
    struct addrinfo *serverInfo;            //Used to point to results
    int option_value;                       //Option value will be set to 1 to set REUSEADDR for SO_Socket
    std::string  headerFormat[3] = {"Method","Path","Version"};  //format of expected header. Used to parse header GET Request   
    statusResponse statResp;

public:
    enum class Status {STATUS_OK, STATUS_ERROR, STATUS_INVALID_REQUEST, STATUS_FILE_NOT_FOUND};
    std::string port;
    Server(unsigned int maxpending, std::string port);                       
    int serverFD;                                                            //Server file Descriptor
    //public methods (Specify Returning Value, change return value to enum for error handling potentially)
    int initServer(unsigned int maxpending, std::string port);                 //Initialize values for addrinfo
    Status recvRequest(Client *Client);                                           //Receive Header Request
    Status parseHeader(char * buf, std::map<std::string,std::string> *headerMap);  //parseHeader
    Status serverSendBody(Client *Client);                                         //Send Response Body
    Status serverSendHeader(Client *Client);                                        //Send Header
    void cleanup(Client *Client);                                                 //Cleanup Client Context
    void perform();
    Status serverHandler();

    
    
};

#endif