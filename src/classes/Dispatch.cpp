
#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>

Dispatch::Dispatch():password(""){};

Dispatch::Dispatch(std::string pass):password(pass)
{

}

void Dispatch::dispatch(Command cmd, int fd)
{
    if (cmd.getCmd() == "CAP")
        ft_cap(cmd);
    if (cmd.getCmd() == "PASS")
        ft_pass(cmd, fd);
    if (cmd.getCmd() == "NICK")
        ft_nick(cmd, fd);
    if (cmd.getCmd() == "JOIN")
        ft_join(cmd, fd);
}

void Dispatch::ft_cap(Command cmd)
{
    (void)cmd;
}

void Dispatch::ft_pass(Command cmd, int fd)
{
    if (!(cmd.getArgs() == password))
    {
        std::string msg = ":localhost 464 * :Password incorrect\r\n";
        send(fd, msg.c_str(), msg.size(), 0);
        //close(fd);
    }
}
