#include "../../includes/Channel.hpp"


Channel::Channel(std::string topic, std::string name, std::size_t id, std::string key)
:_topic(topic), _name(name), _id(id), _key(key)
{
    return ;
}
Channel::~Channel()
{
    return ;
}

std::string Channel::getTopic() const
{
    return _topic;
}

std::string Channel::getName() const
{
    return _name;
}
std::size_t Channel::getId() const
{
    return _id;
}

std::vector<Client *> Channel::getUsers() const
{
    return users;
}

bool Channel::isOperator(Client* user) const
{
    for (size_t i = 0; i < operators.size(); i++)
    {
        if (operators[i] == user)
            return true;
    }
    return false;
}

bool Channel::isUserInChannel(Client* user) const
{
    for (size_t i = 0; i < users.size(); i++)
    {
        if (users[i] == user)
            return true;
    }
    return false;
}

std::string Channel::getKey() const
{
    return _key;
}

bool Channel::addUser(Client* user)
{
    if (isUserInChannel(user))
        return false;
    users.push_back(user);
    return true;
}

bool Channel::addOperator(Client* user)
{
    if (isOperator(user))
        return false;
    operators.push_back(user);
    return true;
}

void Channel::setTopic(std::string topic)
{
    _topic = topic;
}

void Channel::setProtectTopic(bool protect)
{
    _protectTopic = protect;
}

bool Channel::isTopicProtected() const
{
    return _protectTopic;
}

void Channel::setInviteOnly(bool inviteOnly)
{
    _inviteOnly = inviteOnly;
}

bool Channel::isInviteOnly() const
{
    return _inviteOnly;
}
std::vector<Client *> &Channel::getUserRefs()
{
    return users;
}