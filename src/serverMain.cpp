#ifndef SERVER_H
#include  "server.h"
#endif

int main(int argc, char const *argv[]){
   
    Server socketServer(100,"10.0.0.34","8081");
    socketServer.perform();
    printf("hey");
}