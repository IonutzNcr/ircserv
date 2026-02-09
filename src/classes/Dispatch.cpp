
#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/split.hpp"
#include "../../includes/Channel.hpp"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>

Dispatch::Dispatch(std::string pass, std::vector<Client *>&clients): _password(pass), _clients(clients)
{
    return ;
}

Dispatch::~Dispatch()
{
    return ;
}

void Dispatch::dispatch(Command cmd, int fd)
{
    int choice = 0;

    if (cmd.getCmd() == "CAP")
        ft_cap(cmd, fd);
    if (cmd.getCmd() == "PASS")
        ft_pass(cmd, fd);
    if (cmd.getCmd() == "NICK")
        ft_nick(cmd, fd);
    if (cmd.getCmd() == "USER")
        ft_user(cmd, fd);
    if (cmd.getCmd() == "JOIN")
        ft_join(cmd, fd);
    if (cmd.getCmd() == "MODE")
        ft_mode(cmd, fd);
    if (cmd.getCmd() == "INVITE")
        ft_invite(cmd, fd);
    if (cmd.getCmd() == "TOPIC")
        ft_topic(cmd, fd);
    if (cmd.getCmd() == "PRIVMSG")
        ft_PRIVMSG(cmd, fd);
    /* if (cmd.getCmd() == "PING")
        ft_ping(cmd, fd);  */    
}


bool Dispatch::ft_cap(Command cmd, int fd)
{
    std::string line = cmd.getLine();
    // respond minimally so clients finish capability negotiation
    if (line.find("LS") != std::string::npos) {
        std::string msg = "CAP * LS :\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
    }
    else if (line.find("REQ") != std::string::npos) {
        std::string msg = "CAP * NAK :\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
    }
    else if (line.find("END") != std::string::npos) {
        // nothing needed, client ends negotiation
    }
    return true;
}

//TODO::RPL_TOPIC
//TODO::RPL_NAMREPLY
bool Dispatch::ft_join(Command cmd, int fd)
{
    Client* client = getClientFd(fd);
    if (!client)
        return false;
     if (!client->isRegistered()) // si le client n'es pas register just return false
        return false;
    std::string line = cmd.getLine();
    std::string channelName = line.substr(5); // on stock la string apres le JOIN
    std::vector<std::string> chanXkeys = split(channelName, ' ');
    if (chanXkeys.empty())
    {
        std::string msg = ":server 461 JOIN :Not enough parameters\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
        return false;
    }
    std::string chanNames = chanXkeys[0];
    bool isKey = false;
    if (chanXkeys.size() > 1)
        isKey = true;
    std::string key;
    if (isKey)
        key = chanXkeys[1];
    std::vector<std::string> chanNamesSplit = split(chanNames, ',');
    std::vector<std::string>keysSplit;
    if (isKey)
        keysSplit = split(key, ',');
    for (size_t i = 0; i < chanNamesSplit.size(); i++)
    {
        std::string chanName = chanNamesSplit[i];
        std::string chanKey;
        if (isKey && i < keysSplit.size())
            chanKey = keysSplit[i];
        else
            chanKey = "";
        //creation channel si n'existe pas et ajout user au channel
        Channel *newChan = new Channel("", chanName, i, chanKey); // to fix the id is not really unique
        if (!isChannelExist(chanName)) // TODO:: big problem why ?
        {
            //print les channels existants
            std::cout << "Existing channels: ";
            for (size_t j = 0; j < _channels.size(); j++)
            {
                std::cout << _channels[j]->getName() << " ";
            }
            std::cout << std::endl;
            
            _channels.push_back(newChan);
            newChan->addUser(client);
            newChan->addOperator(client);
            std::string msg = ":" + client->GetNick() + " JOIN " + chanName + "\r\n";
            send(fd, msg.c_str(), msg.length(), 0);
            //RPL_NOTOPIC
            msg = ":server 331 " + client->GetNick() + " " + newChan->getName() + " :No topic is set\r\n";
            send(fd, msg.c_str(), msg.size(), 0);
            //RPL_NAMREPLY
            msg = ":server 353 " + client->GetNick() + " = " + newChan->getName() + " :@" + client->GetNick() + "\r\n";
            send(fd, msg.c_str(), msg.size(), 0);
            //RPL_ENDOFNAMES
            msg = ":server 366 " + client->GetNick() + " " + newChan->getName() + " :End of /NAMES list\r\n";
            send(fd, msg.c_str(), msg.size(), 0);
        }else
        {
            for (size_t j = 0; j < _channels.size(); j++)
            {
                if (_channels[j]->getName() == chanName)
                {
                    std::string  msg;
                    if (_channels[j]->isUserInChannel(client))
                    {
                       /*  msg = "User is already in the channel " + chanName + "\r\n";
                        send(fd, msg.c_str(), msg.length(), 0); */
                        continue; // a voir que faire si deja dans le channel et mauvais key
                    }
                    if (_channels[j]->getKey() != chanKey)
                    {
                        std::string errMsg = ":server 475 " + client->GetNick() + " " + chanName + " :Cannot join channel (+k)\r\n";
                        send(fd, errMsg.c_str(), errMsg.length(), 0);
                        continue;
                    }

                    // Broadcast JOIN to all existing members
                    std::string joinMsg = ":" + client->GetNick() + " JOIN " + chanName + "\r\n";
                    std::vector<Client *> existingUsers = _channels[j]->getUsers();
                    for (std::size_t k = 0; k < existingUsers.size(); k++)
                    {
                        send(existingUsers[k]->GetFd(), joinMsg.c_str(), joinMsg.length(), 0);
                    }
                    // Also send JOIN confirmation to the joining client
                    send(fd, joinMsg.c_str(), joinMsg.length(), 0);
                    _channels[j]->addUser(client);
                    if(_channels[j]->getTopic().empty())
                    {
                        msg = ":server 331 " + client->GetNick() + " " + _channels[j]->getName() + " :No topic is set\r\n";
                        send(fd, msg.c_str(), msg.size(), 0);
                    } else
                    {
                        msg = ":server 332 " + client->GetNick() + " " + _channels[j]->getName() + " :" + _channels[j]->getTopic() + "\r\n"; 
                        send(fd, msg.c_str(), msg.size(), 0);
                    }
                    //RPL_NAMREPLY - list all users with @ prefix for operators
                    msg = ":server 353 " + client->GetNick() + " = " + _channels[j]->getName() + " :";
                    std::vector<Client *> channelUsers = _channels[j]->getUsers();
                    for (std::size_t k = 0; k < channelUsers.size(); k++)
                    {
                        if (_channels[j]->isOperator(channelUsers[k]))
                            msg += "@";
                        msg += channelUsers[k]->GetNick();
                        if (k < channelUsers.size() - 1)
                            msg += " ";
                    }
                    msg += "\r\n";
                    send(fd, msg.c_str(), msg.size(), 0);
                    //RPL_ENDOFNAMES
                    msg = ":server 366 " + client->GetNick() + " " + _channels[j]->getName() + " :End of /NAMES list\r\n";
                    send(fd, msg.c_str(), msg.size(), 0);
                }
            }
        }        
    }
    return true;
}

Client *Dispatch::getClientFd(int fd_client)
{
    for (size_t i = 0; i < _clients.size(); ++i) {
            if (_clients[i]->GetFd() == fd_client)
                return (_clients[i]);
        }
    return (NULL);
}

void    Dispatch::tryRegister(Client* client)
{
    if (client->isRegistered())
        return ;
    if (client->isAuthenticated() && !client->GetNick().empty() && !client->GetUser().empty()) {
        client->setRegistered(true);
        std::string msg = ":server 001 " + client->GetNick() + " :Welcome to the IRC server!\r\n";
        send(client->GetFd(), msg.c_str(), msg.length(), 0);
    }
    if (!client->isAuthenticated() && !client->GetNick().empty() && !client->GetUser().empty()) {
        std::string msg2 = "464 * :Password incorrect\r\n";
        send(client->GetFd(), msg2.c_str(), msg2.length(), 0);
        // a voir si on deconnecte le client ou pas
    }
}

bool Dispatch::isChannelExist(std::string chanName)
{
    for (size_t i = 0; i < _channels.size(); i++)
    {
        if (_channels[i]->getName() == chanName)
            return true;
    }
    return false;
}

/* bool Dispatch::ft_ping(Command cmd, int fd)
{
    std::string line = cmd.getLine();
    std::string token;
    if (line.size() > 5)
        token = line.substr(5);  // après "PING "
    else
        token = "server";
    
    // Trim whitespace
    size_t start = token.find_first_not_of(" \t\r\n");
    size_t end = token.find_last_not_of(" \t\r\n");
    if (start != std::string::npos && end != std::string::npos)
        token = token.substr(start, end - start + 1);
    
    std::string reply = ":server PONG server :" + token + "\r\n";
    send(fd, reply.c_str(), reply.length(), 0);
    return true;
} */

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

Channel*    Dispatch::getChannel(std::string target)
{
    for (size_t i = 0; i < _channels.size(); i++) {
        if (_channels[i]->getName() == target)
            return (_channels[i]);
    }
    return (NULL);
}

bool Dispatch::ft_mode(Command cmd, int fd)
{
    Client* client = getClientFd(fd);
    if (!client)
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
        if (target[0] == '#') {
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
    
    if (target[0] == '#') {
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
        std::string msg = ":server 472 " + client->GetNick() + " " + target + " :is unknown mode char to me\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
        return false;
    }
    return true;
}

bool Dispatch::ft_invite(Command cmd, int fd)
{
    Client* client = getClientFd(fd);
    if (!client)
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

bool Dispatch::ft_topic(Command cmd, int fd)
{
    Client* client = getClientFd(fd);
    if (!client)
        return false;
    
    std::string line = cmd.getLine();
    std::vector<std::string> tokens = split(line, ' ');
    if (tokens.size() < 2) {
        std::string msg = ":server 461 " + client->GetNick() + " TOPIC :Not enough parameters\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
        return false;
    }
    std::string channelName = tokens[1];
    if (channelName.empty())
    {
        std::string msg = ":server 461 " + client->GetNick() + " TOPIC :Not enough parameters\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
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

    if (cmd.getTrailing().empty())
    {
        if (channel->getTopic().empty())
        {
            std::string msg = ":server 331 " + client->GetNick() + " " + channelName + " :No topic is set\r\n";
            send(fd, msg.c_str(), msg.length(), 0);
        }
        else
        {
            std::string msg = ":server 332 " + client->GetNick() + " " + channelName + " :" + channel->getTopic() + "\r\n";
            send(fd, msg.c_str(), msg.length(), 0);
        }
        return true;
    }

    if (channel->isTopicProtected() && !channel->isOperator(client)) {
        std::string errMsg = ":server 482 " + client->GetNick() + " " + channel->getName() + " :You're not channel operator\r\n";
        send(fd, errMsg.c_str(), errMsg.length(), 0);
        return false;
    }

    std::string newTopic = cmd.getTrailing();
    channel->setTopic(newTopic);

    std::string topicMsg = ":" + client->GetNick() + " TOPIC " + channelName + " :" + newTopic + "\r\n";

    std::vector<Client*> users = channel->getUsers();
    for (size_t i = 0; i < users.size(); i++)
        send(users[i]->GetFd(), topicMsg.c_str(), topicMsg.length(), 0);

    return true;
}

int	Dispatch::findClient(std::string nick)
{
	for(size_t i = 0; i < _clients.size(); i++){
		if (_clients[i]->GetNick() == nick) {
                return (_clients[i]->GetFd());
            }
	}
	return (-1);
}