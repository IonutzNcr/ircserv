#include "../../includes/RplReply.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/Debugger.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <vector>

void RplReply::RPL_NOTOPIC(Client &client, Channel &chan, int fd)
{
    std::string msg = ":server 331 " + client.GetNick() + " " + chan.getName() + " :No topic is set\r\n";
    Debugger::storeLog(2, msg);
    send(fd, msg.c_str(), msg.size(), 0);
}

void RplReply::RPL_TOPIC(Client &client, Channel &chan, int fd)
{
    std::string msg = ":server 332 " + client.GetNick() + " " + chan.getName() + " :" + chan.getTopic() + "\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    Debugger::storeLog(2, msg);
}

void RplReply::RPL_NAMREPLY(Client &client, Channel &chan, int fd)
{
    std::string msg = ":server 353 " + client.GetNick() + " = " + chan.getName() + " :";
    std::vector<Client *> channelUsers = chan.getUsers();
    for (std::size_t index = 0; index < channelUsers.size(); index++)
    {
        if (chan.isOperator(channelUsers[index]))
            msg += "@";
        msg += channelUsers[index]->GetNick();
        if (index < channelUsers.size() - 1)
            msg += " ";
    }
    msg += "\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    Debugger::storeLog(2, msg);
}

void RplReply::RPL_ENDOFNAMES(Client &client, Channel &chan, int fd)
{
    std::string msg = ":server 366 " + client.GetNick() + " " + chan.getName() + " :End of /NAMES list\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    Debugger::storeLog(2, msg);
}

void RplReply::ERR_NEEDMOREPARAMS(Client &client, const std::string &command, int fd)
{
    (void)client;
    std::string msg = ":server 461 " + command + " :Not enough parameters\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    Debugger::storeLog(2, msg);
}

void RplReply::ERR_USERONCHANNEL(Client &client, const std::string &targetNick, Channel &chan, int fd)
{
    std::string msg = ":server 443 " + client.GetNick() + " " + targetNick + " " + chan.getName() + " :is already on channel\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    Debugger::storeLog(2, msg);
}

void RplReply::ERR_INVITEONLYCHAN(Client &client, Channel &chan, int fd)
{
    std::string msg = ":server 473 " + client.GetNick() + " " + chan.getName() + " :Cannot join channel (+i)\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    Debugger::storeLog(2, msg);
}

void RplReply::ERR_BADCHANNELKEY(Client &client, Channel &chan, int fd)
{
    std::string msg = ":server 475 " + client.GetNick() + " " + chan.getName() + " :Cannot join channel (+k)\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    Debugger::storeLog(2, msg);
}

void RplReply::ERR_NOSUCHCHANNEL(Client &client, const std::string &channelName, int fd)
{
    std::string msg = ":server 403 " + client.GetNick() + " " + channelName + " :No such channel\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    Debugger::storeLog(2, msg);
}

void RplReply::ERR_CHANOPRIVSNEEDED(Client &client, Channel &chan, int fd)
{
    std::string msg = ":server 482 " + client.GetNick() + " " + chan.getName() + " :You're not channel operator\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    Debugger::storeLog(2, msg);
}

void RplReply::ERR_USERNOTINCHANNEL(Client &client, const std::string &targetNick, Channel &chan, int fd)
{
    std::string msg = ":server 441 " + client.GetNick() + " " + targetNick + " " + chan.getName() + " :They aren't on that channel\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    Debugger::storeLog(2, msg);
}

void RplReply::ERR_CHANNELISFULL(Client &client, Channel &chan, int fd)
{
    std::string msg = ":server 471" +  client.GetNick() + " " +  chan.getName()  + " " + " :Cannot join channel (+l)\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    Debugger::storeLog(2, msg);
}