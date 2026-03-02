
#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/split.hpp"
#include "../../includes/Channel.hpp"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>

bool Dispatch::ft_invite(Command cmd, int fd)
{
    Client* client = getClientFd(fd);
    if (!client)
        return false;
    if (!client->isRegistered())
        return true;
    std::string line = cmd.getLine();
    std::vector<std::string> tokens = split(line, ' ');
    if (tokens.size() < 3) {
        std::string msg = ":server 461 " + client->GetNick() + " INVITE :Not enough parameters\r\n";
        sendAll(fd, msg);
        return true;
    }
    std::string targetNick = tokens[1];
    std::string channelName = tokens[2];
    
    Client* targetClient = NULL;
    for (size_t i = 0; i < _clients.size(); i++) {
        if (ircCaseEqual(_clients[i]->GetNick(), targetNick)) {
            targetClient = _clients[i];
            break;
        }
    }

    if (!targetClient) {
        std::string errMsg = ":server 401 " + client->GetNick() + " " + targetNick + " :No such nick\r\n";
        sendAll(fd, errMsg);
        return true;
    }
    
    Channel* channel = NULL;
    for (size_t i = 0; i < _channels.size(); i++) {
        if (ircCaseEqual(_channels[i]->getName(), channelName)) {
            channel = _channels[i];
            break;
        }
    }

    if (!channel) {
        std::string errMsg = ":server 403 " + client->GetNick() + " " + channelName + " :No such channel\r\n";
        sendAll(fd, errMsg);
        return true;
    }

    if (!channel->isUserInChannel(client))
    {
        std::string errMsg = ":server 442 " + client->GetNick() + " " + channel->getName() + " :You're not on that channel\r\n";
        sendAll(fd, errMsg);
        return true;
    }

    if (!channel->isOperator(client)) {
        std::string errMsg = ":server 482 " + client->GetNick() + " " + channel->getName() + " :You're not channel operator\r\n";
        sendAll(fd, errMsg);
        return true;
    }

    if (channel->isUserInChannel(targetClient)) {
        std::string errMsg = ":server 443 " + client->GetNick() + " " + targetClient->GetNick() + " " + channel->getName() + " :is already on channel\r\n";
        sendAll(fd, errMsg);
        return true;
    }
    
    if (!channel->isInvited(targetClient)) {
        channel->addInvited(targetClient);
        std::string inviteMsg = ":" + client->GetNick() + " INVITE " + targetClient->GetNick() + " " + channel->getName() + "\r\n";
        sendAll(targetClient->GetFd(), inviteMsg);
    }

    std::string succesInviteMsg = ":server 341 " + client->GetNick() + " " + targetClient->GetNick() + " " + channel->getName() + "\r\n";
    sendAll(client->GetFd(), succesInviteMsg);
    return true;
}
