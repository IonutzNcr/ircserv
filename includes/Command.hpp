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
        std::string getLine();

        void setLine(std::string line);
        void setCmd(std::string cmd);
        void setPrefix(std::string prefix);
        void setTrailing(std::string trailing);
        void setArgs(std::string arguments);
        void setArgsVector(std::vector<std::string> argsVector)
        {
            _argsVector = argsVector;
        };
        std::vector<std::string> getArgsVector() const
        {
            return _argsVector;
        };

    private:
        std::string _prefix;
        std::string _cmd;
        std::string _args;
        std::string _trailing;
        std::string _line;
        std::vector<std::string> _argsVector;
        //maybe there are others stuff to be concerned i dont know yet
};