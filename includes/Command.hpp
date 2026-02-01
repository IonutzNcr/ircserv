#include <string>
#include <vector>

class Command
{
    public:
        Command();
        Command(std::string cmd,  std::string to, std::string from);
        std::string getCmd() const;
        std::vector<std::string>getArgs() const;
        std::string getDestination() const;
        std::string getSource() const;
        bool addArgs(std::string arg);
    private:
        std::string _cmd;
        std::vector<std::string> _args;
        std::string _to;
        std::string _from;
        //maybe there are others stuff to be concerned i dont know yet
};