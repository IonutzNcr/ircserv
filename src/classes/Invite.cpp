
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
	if (!client->isRegistered()) // si le client n'es pas register just return false
		return false;
    std::string line = cmd.getLine();
    std::vector<std::string> tokens = split(line, ' ');
    if (tokens.size() < 3) {
        std::string msg = ":server 461 " + client->GetNick() + " INVITE :Not enough parameters\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
        return false;
    }
    std::string targetNick = tokens[1];
    std::string channelName = tokens[2];
    
    Client* targetClient = nullptr;
    for (size_t i = 0; i < _clients.size(); i++) {
        if (_clients[i]->GetNick() == targetNick) {
            targetClient = _clients[i];
            break;
        }
    }

    if (!targetClient) {
        std::string errMsg = ":server 401 " + client->GetNick() + " " + targetNick + " :No such nick\r\n";
        send(fd, errMsg.c_str(), errMsg.length(), 0);
        return false;
    }
    
    Channel* channel = nullptr;
    for (size_t i = 0; i < _channels.size(); i++) {
        if (_channels[i]->getName() == channelName) {
            channel = _channels[i];
            break;
        }
    }

    if (!channel) {
        std::string errMsg = ":server 403 " + client->GetNick() + " " + channelName + " :No such channel\r\n";
        send(fd, errMsg.c_str(), errMsg.length(), 0);
        return false;
    }

    if (!channel->isUserInChannel(client))
    {
        std::string errMsg = ":server 442 " + client->GetNick() + " " + channel->getName() + " :You're not on that channel\r\n";
        send(fd, errMsg.c_str(), errMsg.length(), 0);
        return false;
    }

    if (!channel->isOperator(client)) {
        std::string errMsg = ":server 482 " + client->GetNick() + " " + channel->getName() + " :You're not channel operator\r\n";
        send(fd, errMsg.c_str(), errMsg.length(), 0);
        return false;
    }

    if (channel->isUserInChannel(targetClient)) {
        std::string errMsg = ":server 443 " + client->GetNick() + " " + targetClient->GetNick() + " " + channel->getName() + " :is already on channel\r\n";
        send(fd, errMsg.c_str(), errMsg.length(), 0);
        return false;
    }
    
    if (!channel->isInvited(targetClient)) {
        channel->addInvited(targetClient);
        std::string inviteMsg = ":" + client->GetNick() + " INVITE " + targetClient->GetNick() + " " + channel->getName() + "\r\n";
        send(targetClient->GetFd(), inviteMsg.c_str(), inviteMsg.length(), 0);
    }

    std::string succesInviteMsg = ":server 341 " + client->GetNick() + " " + targetClient->GetNick() + " " + channel->getName() + "\r\n";
    send(client->GetFd(), succesInviteMsg.c_str(), succesInviteMsg.length(), 0);
    return true;
}
