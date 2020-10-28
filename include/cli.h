#include <map>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include "stegImage.h"
class cli
{
private:
    static void setAttr(cli *cliInstance);
    static void showInfo(cli *cliInstance);
    static void runCommand(cli *cliInstance);
    typedef void (*cliFunct)(cli *cliInstance); //Function pointer type for CLI interfac
    int parseCommand (std::string command);

public:
    std::map<std::string,cliFunct> map;
    std::vector<std::string> input;
    std::string Kill;
    std::string Conf;
    std::string Gath;
    std::string Shell;
    std::string Exec;
    std::string id;

    static void *cliPerform(void *params);
    void init(cli *cliInstance);
    //Map of commands
    void call_command(cli *cliInstance);
    
    


};