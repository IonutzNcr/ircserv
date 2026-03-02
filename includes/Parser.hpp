#pragma once

#include <string>
#include <iostream>
#include <vector>

#include "Data.hpp"

class Command;

class Parser
{
    public: 
        Parser();
        ~Parser();
        Command getCommand(std::string rawInput);
        void concatData(std::string rawinput, int fd);
        Data &getData(int fd);
        std::string extractcmd(int fd);
        void fill(std::string buff, int fd);
        Command get(int fd);
        std::string getCmdtwo(int fd);
        void clearData(int fd);

    private:
        std::vector<Data> rawsData;
};

