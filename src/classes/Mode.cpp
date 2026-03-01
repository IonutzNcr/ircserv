#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/split.hpp"
#include "../../includes/Channel.hpp"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>

bool Dispatch::setMode(Channel* channel, std::string modeChanges, int fd, std::string target, std::string msg, Client* client, std::vector<std::string> tokens)
{
     if (modeChanges[0] != '+' && modeChanges[0] != '-') {
        std::string msg = ":server 501 " + client->GetNick() + " :Unknown MODE flag\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
        return false;
    }
    for (size_t i = 1; i < modeChanges.size(); i++) 
    {
        char mode = modeChanges[i];
        switch (mode)
        {
        case 't':
            if (modeChanges[0] == '+')
                channel->setProtectTopic(true);
            else
                channel->setProtectTopic(false);
            break;
        case 'i':
            if (modeChanges[0] == '+')
                channel->setInviteOnly(true);
            else
                channel->setInviteOnly(false);
            break;
        case 'k':
            if (modeChanges[0] == '+') {
                if (tokens.size() < 4) {
                    std::string errMsg = ":server 461 MODE :Not enough parameters\r\n";
                    send(fd, errMsg.c_str(), errMsg.length(), 0);
                    return false;
                }
                if (channel->getKey() != "") {
                    std::string errMsg = ":server 467 " + client->GetNick() + " " + channel->getName() + " :Channel key already set\r\n";
                    send(fd, errMsg.c_str(), errMsg.length(), 0);
                    return false;
                }
                std::string newKey = tokens[3];
                channel->setKey(newKey);
            }
            else {
                channel->setKey("");
            }
            break;
        case 'l':
            if (modeChanges[0] == '+') {
                if (tokens.size() < 4) {
                    std::string errMsg = ":server 461 " + client->GetNick() + " " + target + " :Not enough parameters\r\n";
                    send(fd, errMsg.c_str(), errMsg.length(), 0);
                    return false;
                }
                try
                {
                    int maxUsers = std::stoi(tokens[3]);
                    if (maxUsers <= 0) {
                        std::string errMsg = ":server 461 " + client->GetNick() + " " + target + " :Not enough parameters\r\n";
                        send(fd, errMsg.c_str(), errMsg.length(), 0);
                        return false;
                    }
                    channel->setMaxUsers(maxUsers);
                }
                catch(const std::exception& e)
                {
                    std::string errMsg = ":server 461 " + client->GetNick() + " " + target + " :Not enough parameters\r\n";
                    send(fd, errMsg.c_str(), errMsg.length(), 0);
                    return false;
                }
            }
            else {
                channel->setMaxUsers(0);
            }
            break;
        case 'o':
            if (tokens.size() < 4) {
                std::string errMsg = ":server 401 " + client->GetNick() + " " + target + " :No such nick/channel\r\n";
                send(fd, errMsg.c_str(), errMsg.length(), 0);
                return false;
            }
            if (modeChanges[0] == '+') {
                std::string targetNick = tokens[3];
                Client* targetClient = nullptr;
                for (size_t j = 0; j < _clients.size(); j++) {
                    if (_clients[j]->GetNick() == targetNick) {
                        targetClient = _clients[j];
                        break;
                    }
                }
                if (!targetClient) {
                    std::string errMsg = ":server 401 " + client->GetNick() + " " + targetNick + " :No such nick\r\n";
                    send(fd, errMsg.c_str(), errMsg.length(), 0);
                    return false;
                }
                if (!channel->isUserInChannel(targetClient)) {
                    std::string errMsg = ":server 441 " + client->GetNick() + " " + targetNick + " " + channel->getName() + " :They aren't on that channel\r\n";
                    send(fd, errMsg.c_str(), errMsg.length(), 0);
                    return false;
                }
                channel->addOperator(targetClient);
            }
            else {
                std::string targetNick = tokens[3];
                Client* targetClient = nullptr;
                for (size_t j = 0; j < _clients.size(); j++) {
                    if (_clients[j]->GetNick() == targetNick) {
                        targetClient = _clients[j];
                        break;
                    }
                }
                if (!targetClient) {
                    std::string errMsg = ":server 401 " + client->GetNick() + " " + targetNick + " :No such nick\r\n";
                    send(fd, errMsg.c_str(), errMsg.length(), 0);
                    return false;
                }
                if (!channel->isUserInChannel(targetClient)) {
                    std::string errMsg = ":server 441 " + client->GetNick() + " " + targetNick + " " + channel->getName() + " :They aren't on that channel\r\n";
                    send(fd, errMsg.c_str(), errMsg.length(), 0);
                    return false;
                }
                channel->removeOperator(targetClient);
            } 
            break;
        
        default:
            std::string errMsg = ":server 472 " + client->GetNick() + " " + modeChanges.substr(1, 1) + " :is unknown mode char to me\r\n";
            send(fd, errMsg.c_str(), errMsg.length(), 0);
            return false;
        }
    }
    std::string modeMsg = ":" + client->GetNick() + " MODE " + target + " " + modeChanges;
    for (size_t i = 3; i < tokens.size(); i++) {
        modeMsg += " " + tokens[i];
    }
    modeMsg += "\r\n";
    std::vector<Client*> users = channel->getUsers();
    for (size_t j = 0; j < users.size(); j++) {
        send(users[j]->GetFd(), modeMsg.c_str(), modeMsg.length(), 0);
    }
    return true;
}

bool Dispatch::ft_mode(Command cmd, int fd)
{
    Client* client = getClientFd(fd);
    if (!client)
        return false;
	if (!client->isRegistered()) // si le client n'es pas register just return false
		return false;
    std::string line = cmd.getLine();
    std::vector<std::string> tokens = split(line, ' ');
    if (tokens.size() < 2) {
        std::string msg = ":server 461 " + client->GetNick() + " MODE :Not enough parameters\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
        return false;
    }
    if (tokens.size() == 2) {
        std::string target = tokens[1];
        if (target[0] == '#' || target[0] == '&') {
            Channel* channel = nullptr;
            for (size_t i = 0; i < _channels.size(); i++) {
                if (_channels[i]->getName() == target) {
                    channel = _channels[i];
                    break;
                }
            }
            if (!channel) {
                std::string msg = ":server 403 " + client->GetNick() + " " + target + " :No such channel\r\n";
                send(fd, msg.c_str(), msg.length(), 0);
                return false;
            }
            std::string options = "";
            if (channel->isTopicProtected())
                options += "t";
            if (channel->isInviteOnly())
                options += "i";
            if (!channel->getKey().empty())
                options += "k";
            if (channel->getMaxUsers() > 0)
                options += "l";
            std::string msg = ":server 324 " + client->GetNick() + " " + channel->getName() + " +" + options + "\r\n";
            send(fd, msg.c_str(), msg.length(), 0);
            std::string msg2 = ":server 329 " + client->GetNick() + " " + channel->getName() + " " + std::to_string(channel->getId()) + "\r\n";
            send(fd, msg2.c_str(), msg2.length(), 0);
        }
        else {
            std::string msg = ":server 472 " + client->GetNick() + " " + target + " :is unknown mode char to me\r\n";
            send(fd, msg.c_str(), msg.length(), 0);
            return false;
        }
        return true;
    }
    std::string target = tokens[1];
    std::string modeChanges = tokens[2];
    
    if (target[0] == '#' || target[0] == '&') {
        Channel* channel = nullptr;
        for (size_t i = 0; i < _channels.size(); i++) {
            if (_channels[i]->getName() == target) {
                channel = _channels[i];
                break;
            }
        }
        if (!channel) {
            std::string msg = ":server 403 " + client->GetNick() + " " + target + " :No such channel\r\n";
            send(fd, msg.c_str(), msg.length(), 0);
            return false;
        }
        if (!channel->isOperator(client)) {
            std::string msg = ":server 482 " + client->GetNick() + " " + channel->getName() + " :You're not channel operator\r\n";
            send(fd, msg.c_str(), msg.length(), 0);
            return false;
        }
        if (setMode(channel, modeChanges, fd, target, line, client, tokens) == false) 
            return false;
    }
    else {
        // std::string msg = ":server 472 " + client->GetNick() + " " + modeChanges + " :is unknown mode char to me\r\n";
        // send(fd, msg.c_str(), msg.length(), 0);
        return false;
    }
    return true;
}