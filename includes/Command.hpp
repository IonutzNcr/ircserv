#include <string>
#include <vector>

class Command
{
    public:
        Command();
        Command(std::string prefix, std::string command, std::string trailing);
        std::string getCmd() const;
        std::string getPrefix() const;
        std::string getTrailing() const;
        std::string getArgs() const;


        void setCmd(std::string cmd);
        void setPrefix(std::string prefix);
        void setTrailing(std::string trailing);
        void setArgs(std::string arguments);


    private:
        std::string _prefix;
        std::string _cmd;
        std::string _args;
        std::string _trailing;
        //maybe there are others stuff to be concerned i dont know yet
};