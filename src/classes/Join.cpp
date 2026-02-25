
#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/split.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/RplReply.hpp"
#include "../../includes/Debugger.hpp"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>

bool Dispatch::ft_join(Command cmd, int fd)
{
    Client* client = getClientFd(fd);
    RplReply replies;
    if (!client)
        return false;
     if (!client->isRegistered()) // si le client n'es pas register just return false
        return false;
    std::string line = cmd.getLine();
    std::string channelName = line.substr(5); // on stock la string apres le JOIN
    std::vector<std::string> chanXkeys = split(channelName, ' ');

    if (chanXkeys.empty())
    {
        replies.ERR_NEEDMOREPARAMS(*client, "JOIN", fd);
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
        if (!isChannelExist(chanName)) // TODO:: big problem why ?
        {
            Channel *newChan = createChannel("", chanName, i, chanKey, client);
            broadcastJoin(newChan, client);
            sendTopic(client, newChan);
            sendList(newChan, client);
        }
        else
        {
            //get channel
            Channel *channel = getChannel(chanName);
            std::string  msg;
            if (channel->isUserInChannel(client))
            {
                replies.ERR_USERONCHANNEL(*client, client->GetNick(), *channel, fd);
                continue; // a voir que faire si deja dans le channel et mauvais key
            }
            if (channel->getMaxUsers() > 0 && channel->getUsers().size() >= channel->getMaxUsers())
            {
                replies.ERR_CHANNELISFULL(*client, *channel, fd);
                continue;
            }
            if (channel->isInviteOnly() && !channel->isInvited(client))
            {
                replies.ERR_INVITEONLYCHAN(*client, *channel, fd);
                continue;
            }
            if (channel->getKey() != chanKey)
            {
                replies.ERR_BADCHANNELKEY(*client, *channel, fd);
                continue;
            }
            channel->addUser(client);
            broadcastJoin(channel, client);
            sendTopic(client, channel);
            sendList(channel, client);
        }        
    }
    return true;
}

Channel *Dispatch::createChannel(std::string topic, std::string name,std::size_t id, std::string key, Client *client)
{
    Channel *newChan = new Channel(topic, name, id, key); 
    newChan->setKey(key);
    _channels.push_back(newChan);
    newChan->addUser(client);
    newChan->addOperator(client);
    return newChan;
}

void Dispatch::sendTopic(Client *client, Channel *channel)
{
    RplReply replies;
    if (!channel->getTopic().empty())
    {
        replies.RPL_TOPIC(*client, *channel, client->GetFd());
    }
}

void Dispatch::broadcastJoin(Channel *channel, Client *client)
{
    std::string joinMsg = ":" + client->GetNick() + " JOIN " + channel->getName() + "\r\n";
    std::vector<Client *> existingUsers = channel->getUsers();
    for (std::size_t k = 0; k < existingUsers.size(); k++)
    {
        Debugger::storeLog(2,joinMsg + " to " + existingUsers[k]->GetNick());
        send(existingUsers[k]->GetFd(), joinMsg.c_str(), joinMsg.length(), 0);
    }
}

void Dispatch::sendList(Channel *channel, Client *client)
{
    RplReply replies;
    replies.RPL_NAMREPLY(*client, *channel, client->GetFd());
    replies.RPL_ENDOFNAMES(*client, *channel, client->GetFd());
}