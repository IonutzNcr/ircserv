
#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/split.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/RplReply.hpp"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>

//KICK <channel>{,<channel>} <user> [<comment>] 
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
        std::vector <std::string> split_channels = split(tokens[0], ',');
        std::string targetNick = tokens[1];

        std::string reason = (cmd.getTrailing().empty() ? "No reason" : cmd.getTrailing());
        
        // Trouver le client cible une seule fois
        Client* targetClient = NULL;
        for (size_t k = 0; k < _clients.size(); k++) {
            if (_clients[k]->GetNick() == targetNick) {
                targetClient = _clients[k];
                break;
            }
        }

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
                continue;
            }

            if (!channel->isOperator(client))
            {
                replies.ERR_CHANOPRIVSNEEDED(*client, *channel, fd);
                continue;
            }

            if (!targetClient || !channel->isUserInChannel(targetClient)) {
                replies.ERR_USERNOTINCHANNEL(*client, targetNick, *channel, fd);
                continue;
            }
            
            // Broadcast KICK message to all channel members
            std::string user = client->GetUser();
            std::string kickMsg = ":" + client->GetNick() + "!" + user + "@localhost" + " KICK " + split_channels[i] + " " + targetNick + " :" + reason + "\r\n";
            std::vector<Client *> channelUsers = channel->getUsers();
            for (std::size_t j = 0; j < channelUsers.size(); j++)
            {
                send(channelUsers[j]->GetFd(), kickMsg.c_str(), kickMsg.length(), 0);
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