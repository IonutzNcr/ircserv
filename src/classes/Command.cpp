#include "../../includes/Command.hpp"

Command::Command():_prefix(""), _cmd(""), _trailing("")
{}

Command::Command(std::string prefix, std::string cmd, std::string trailing)
:_prefix(prefix), _cmd(cmd), _trailing(trailing)
{}

void Command::setCmd(std::string cmd)
{
    _cmd = cmd;
}
void Command::setPrefix(std::string prefix)
{
    _prefix = prefix;
}
void Command::setTrailing(std::string trailing)
{
    _trailing = trailing;
}

void Command::setArgs(std::string args)
{
    _args = args;
}

std::string Command::getCmd() const 
{
    return _cmd;
}

std::string Command::getArgs() const
{
    return _args;
}

std::string Command::getPrefix() const
{
    return _prefix;
}

std::string Command::getTrailing() const
{
    return _trailing;
}

std::string Command::getLine()
{
    return _line;
}

void Command::setLine(std::string line)
{
    _line = line;
}


