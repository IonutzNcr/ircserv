
#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/split.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/RplReply.hpp"
#include <string>
#include <sys/types.h>
#include "../../includes/Server.hpp"
#include <sys/socket.h>

static bool isValidChannelName(const std::string& name)
{
    if (name.empty() || name.size() > 50)
        return false;
    if (name[0] != '#' && name[0] != '&')
        return false;
    for (size_t i = 1; i < name.size(); i++)
    {
        char c = name[i];
        if (c == ' ' || c == '\x07' || c == ',' || c == '\0')
            return false;
    }
    return true;
}

bool Dispatch::ft_join(Command cmd, int fd)
{
    Client* client = getClientFd(fd);
    RplReply replies;
    if (!client)
        return false;
     if (!client->isRegistered())
    {
        return (false);
    }
    std::string channelName = cmd.getArgs();
    if (channelName.empty())
    {
        replies.err_needmoreparams(*client, "JOIN", fd);
        return true;
    }
    std::vector<std::string> chanXkeys = split(channelName, ' ');

    if (chanXkeys.empty())
    {
        replies.err_needmoreparams(*client, "JOIN", fd);
        return true;
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
        if (!isValidChannelName(chanName))
        {
            replies.err_nosuchchannel(*client, chanName, fd);
            continue;
        }
      
        if (!isChannelExist(chanName))
        {
            Channel *newChan = createChannel("", chanName, chanKey, client);
            broadcastJoin(newChan, client);
            sendTopic(client, newChan);
            sendList(newChan, client);
        }
        else
        {
            Channel *channel = getChannel(chanName);
            std::string  msg;
            if (channel->isUserInChannel(client))
            {
                replies.err_useronchannel(*client, client->GetNick(), *channel, fd);
                continue;
            }
            if (channel->getMaxUsers() > 0 && channel->getUsers().size() >= channel->getMaxUsers())
            {
                replies.err_channelisfull(*client, *channel, fd);
                continue;
            }
            if (channel->isInviteOnly() && !channel->isInvited(client))
            {
                replies.err_inviteonlychan(*client, *channel, fd);
                continue;
            }
            if (!channel->getKey().empty() && channel->getKey() != chanKey)
            {
                replies.err_badchannelkey(*client, *channel, fd);
                continue;
            }
            channel->addUser(client);
            broadcastJoin(channel, client);
            sendTopic(client, channel);
            sendList(channel, client);
            channel->removeInvited(client);
        }        
    }
    return true;
}

Channel *Dispatch::createChannel(std::string topic, std::string name, std::string key, Client *client)
{
    Channel *newChan = new Channel(topic, name, key); 
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
        replies.rpl_topic(*client, *channel, client->GetFd());
    }
}

void Dispatch::broadcastJoin(Channel *channel, Client *client)
{
    std::string user = client->GetUser().empty() ? "user" : client->GetUser();
    std::string host = client->GetIpAdd().empty() ? "localhost" : client->GetIpAdd();
    std::string joinMsg = ":" + client->GetNick() + "!" + user + "@" + host + " JOIN :" + channel->getName() + "\r\n";
    std::vector<Client *> existingUsers = channel->getUsers();
    for (std::size_t k = 0; k < existingUsers.size(); k++)
    {
        sendAll(existingUsers[k]->GetFd(), joinMsg);
    }
}

void Dispatch::sendList(Channel *channel, Client *client)
{
    RplReply replies;
    replies.rpl_namreply(*client, *channel, client->GetFd());
    replies.rpl_endofnames(*client, *channel, client->GetFd());
}