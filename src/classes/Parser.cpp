

#include "../../includes/Parser.hpp"
#include <iostream>
#include "../../includes/Command.hpp"

Parser::Parser(){}
Parser::~Parser(){}

Command Parser::getCommand(std::string cmdline)
{
    Command cmd;
    try
    {
        std::size_t index = cmdline.find(":");
        std::size_t end;
        if (index == std::string::npos)
        {
            cmd.setPrefix("");
            index = 0;
            end = 0;
        }
        else
        {
            end = cmdline.find(" ");
            cmd.setPrefix(cmdline.substr(index, end));
            cmdline = cmdline.substr(end);
        }
        index = end;
        end = cmdline.find(" ");
        std::cout << "je passe par la \n";
        if (end == std::string::npos)
        {
            cmd.setCmd("");
            throw std::runtime_error("No command found");
        }
        else
        {
            cmd.setCmd(cmdline.substr(index, end));
            cmdline = cmdline.substr(end);
        }
        index = end;
        end = cmdline.find(":");
        if (end == std::string::npos)
        {
            cmd.setArgs(cmdline); 
            cmd.setTrailing("");
            return (cmd);
        }
        else
        {
            cmd.setArgs(cmdline.substr(index, end));
            cmdline = cmdline.substr(end);
        }
        cmd.setTrailing(cmdline);

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        
        cmd.setCmd("");
        cmd.setPrefix("");
        cmd.setTrailing("");
    }
    return (cmd);
}


Data &Parser::getData(int fd)
{
    std::vector <Data>::iterator dt = rawsData.begin();
    for (;dt != rawsData.end(); dt++)
    {
        if (dt->fd == fd)
            return (*dt);
    }
    //si je trouve pas c que ya pas eu de data creer pour l'instant...
    Data data(fd, "", 0);
    rawsData.push_back(data);
    dt = rawsData.begin();
    for (;dt != rawsData.end(); dt++)
    {
        if (dt->fd == fd)
            return (*dt);
    }
    throw std::runtime_error("Something hoorible happened in getData method");
}

void Parser::concatData(std::string rawData, int fd)
{
    Data &data = getData(fd);
    data.data = std::string(data.data + rawData);
}

std::string Parser::extractcmd(int fd)
{
    Data &data = getData(fd);
    std::size_t index = data.data.find("\r\n");
    /* std::cout << "in extract characters " << ((data.data)[0] == '\n') << std::endl;
    std::cout << "in extract: " << data.data << std::endl; */
    if (index == std::string::npos)
        return std::string("");
    std::string cmdline = data.data.substr(0, index);
    std::cout << "in extract cmd : " << data.data << std::endl;
    std::string newData = data.data.substr(index + 2);
    //some print to see if is working ...
    /* std::cout << "cmd line = " << cmdline << std::endl;
    std::cout << "rest line = " << newData << std::endl; */

    data.data = newData;
    
    return (cmdline);
    
}

Command Parser::get(int fd)
{
        std::string data = extractcmd(fd);
        Command cmd = getCommand(data);

        return (cmd);
}

void Parser::fill(std::string buff, int fd)
{
    concatData(buff, fd);
}