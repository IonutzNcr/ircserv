#include <string>
#include <iostream>
#include "Command.hpp"

#include "Data.hpp"


class Parser
{
    public: 
        Parser();
        ~Parser();
        Command getCommand(std::string rawInput, int fd);
        void concatData(std::string rawinput, int fd);
        void findRL(std::string rawInput)
        {
            int i = 0;
            int r = 0;
            int n = 0;
            while (rawInput[i])
            {
                if (rawInput[i] == '\r')
                    r++;
                if (rawInput[i] == '\n')
                    n++;
                i++;
            }
            std::cout << "r n " << r << " " << n << " \n"; 
        };

    private:
        std::vector<Data> rawsData;
};

