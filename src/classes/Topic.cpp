
#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/split.hpp"
#include "../../includes/Channel.hpp"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>

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