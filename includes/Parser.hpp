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

        int findRN(std::string rawInput)
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
                if (r == 1 && n == 1)
                    return (i);
                i++;
            }
            std::cout << "r n " << r << " " << n << " \n"; 
            return (i);
        };

    private:
        std::vector<Data> rawsData;
};

