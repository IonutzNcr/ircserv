

#include "../../includes/Parser.hpp"
#include <iostream>


Parser::Parser(){}
Parser::~Parser(){}

Command Parser::getCommand(std::string rawInput, int fd)
{
    std::cout << "FD" << fd << " " << rawInput << std::endl;
    
    if(rawsData.size() == 0 || !findFd(fd))
    {
        Data data(fd, rawInput, 0);
        rawsData.push_back(data);
        std::string cmdline;
        while((cmdline = extractCmd(cmdline)))
        {
            std::string *split = std::split(cmdline);
            Command command;
            command.setCmd(split[0]);
            command.setArg(split[1]);// ici je devrais boucler ou en tout cas la j'ai besoin de savoir le format
            Commander::execute(command);
        }
    }
        
    /* findRL(rawInput);
    std::cout << "-----\n"; */
    
    return (command);
}