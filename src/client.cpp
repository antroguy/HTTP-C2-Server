#include "client.h"

void Client::initClient(){
    this->clientFD = 0;
    this->clientLen =0;
    this->status = 0;
    memset(&this->clientAddress,0,sizeof(this->clientAddress));
}
