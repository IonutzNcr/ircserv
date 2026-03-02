
#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/split.hpp"
#include "../../includes/Channel.hpp"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>

bool Dispatch::ft_part(Command cmd, int fd)
{
    Client* client = getClientFd(fd);
    if (!client)
        return false;
    if (!client->isRegistered())
        return false;

    // PART #chan1,#chan2 :reason
    // Extraire les channels et la raison
    std::string args = cmd.getArgs();
    std::string reason = cmd.getTrailing();
    
    if (args.empty())
    {
        std::string msg = ":server 461 " + client->GetNick() + " PART :Not enough parameters\r\n";
        sendAll(fd, msg);
        return true;
    }

    // Si pas de trailing, raison par défaut
    if (reason.empty())
        reason = client->GetNick();

    // Splitter les channels par virgule
    std::vector<std::string> channels = split(args, ',');

    for (size_t i = 0; i < channels.size(); i++)
    {
        std::string channelName = channels[i];
        
        // Trouver le channel
        Channel* channel = getChannel(channelName);
        if (!channel)
        {
            std::string errMsg = ":server 403 " + client->GetNick() + " " + channelName + " :No such channel\r\n";
            sendAll(fd, errMsg);
            continue;
        }

        // Vérifier que le client est dans le channel
        if (!channel->isUserInChannel(client))
        {
            std::string errMsg = ":server 442 " + client->GetNick() + " " + channelName + " :You're not on that channel\r\n";
            sendAll(fd, errMsg);
            continue;
        }

        // Construire le message PART
        std::string user = client->GetUser().empty() ? "user" : client->GetUser();
        std::string host = client->GetIpAdd().empty() ? "localhost" : client->GetIpAdd();
        std::string partMsg = ":" + client->GetNick() + "!" + user + "@" + host + " PART " + channel->getName() + " :" + reason + "\r\n";

        // Envoyer le message PART à tous les membres du channel (y compris le client qui part)
        std::vector<Client*> users = channel->getUsers();
        for (size_t j = 0; j < users.size(); j++)
        {
            sendAll(users[j]->GetFd(), partMsg);
        }

        // Retirer le client du channel
        channel->removeUser(client);
        channel->removeOperator(client);
        channel->removeInvited(client);

        // Si le channel est vide, le supprimer
        if (channel->getUserRefs().empty())
        {
            channel->removeInvitedAll();
            removeChannel(channel);
        }
    }

    return true;
}
