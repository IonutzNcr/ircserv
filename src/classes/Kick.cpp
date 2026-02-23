
#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/split.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/RplReply.hpp"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>

//TODO:: <channel>{,<channel>} <user>{,<user>} [<comment>] 
bool Dispatch::ft_kick(Command cmd, int fd)
{
        Client* client = getClientFd(fd);
        RplReply replies;
        if (!client)
            return false;
        if (!client->isRegistered())
            return false;
        std::string line = cmd.getArgs();
        std::vector<std::string> tokens = split(line, ' ');
        
        if (tokens.size() < 2) {
            replies.ERR_NEEDMOREPARAMS(*client, "KICK", fd);
            return false;
        }
        //pb ici 1000%
        std::vector <std::string> split_channels = split(tokens[0], ',');
        std::vector <std::string> split_target = split(tokens[1], ','); // 1 element

        std::string reason = (cmd.getTrailing().empty() ? "No reason" : cmd.getTrailing());
        
        //TODO:: il reconnait pas le channel pk?
        for(size_t i = 0; i < split_channels.size(); i++)
        {
            Channel* channel = NULL;
            for (size_t k = 0; k < _channels.size(); k++)
            {
                if (_channels[k]->getName() == split_channels[i])
                {
                    channel = _channels[k];
                    break;
                }
            }

            if (!channel)
            {
                replies.ERR_NOSUCHCHANNEL(*client, split_channels[i], fd);
                return false;
            }

            if (!channel->isOperator(client))
            {
                replies.ERR_CHANOPRIVSNEEDED(*client, *channel, fd);
                return false;
            }
            Client* targetClient = NULL;
            for (size_t i = 0; i < _clients.size(); i++) {
                if (_clients[i]->GetNick() == split_target[0]) {
                    targetClient = _clients[i];
                    break;
                }
            }

            if (!targetClient || !channel->isUserInChannel(targetClient)) {
                replies.ERR_USERNOTINCHANNEL(*client, split_target[0], *channel, fd);
                return false;
            }
            
            // Broadcast KICK message to all channel members
            std::string kickMsg = ":" + client->GetNick() + " KICK " + split_channels[i] + " " + split_target[0] + " :" + reason + "\r\n";
            std::vector<Client *> channelUsers = channel->getUsers();
            for (std::size_t i = 0; i < channelUsers.size(); i++)
            {
                send(channelUsers[i]->GetFd(), kickMsg.c_str(), kickMsg.length(), 0);
            }

            // Remove target client from channel
            std::vector<Client *> &users = channel->getUserRefs();
            for (std::vector<Client *>::iterator it = users.begin(); it != users.end(); ++it)
            {
                if (*it == targetClient) {
                    users.erase(it);
                    break;
                }
            }
        }
        
        return true;
}