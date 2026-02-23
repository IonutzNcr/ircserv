

#pragma once

#include <string>

class Client;
class Channel;

class RplReply
{
    public:
        void RPL_NOTOPIC(Client &client, Channel &chan, int fd);
        void RPL_TOPIC(Client &client, Channel &chan, int fd);
        void RPL_NAMREPLY(Client &client, Channel &chan, int fd);
        void RPL_ENDOFNAMES(Client &client, Channel &chan, int fd);

        void ERR_NEEDMOREPARAMS(Client &client, const std::string &command, int fd);
        void ERR_USERONCHANNEL(Client &client, const std::string &targetNick, Channel &chan, int fd);
        void ERR_INVITEONLYCHAN(Client &client, Channel &chan, int fd);
        void ERR_BADCHANNELKEY(Client &client, Channel &chan, int fd);
        void ERR_NOSUCHCHANNEL(Client &client, const std::string &channelName, int fd);
        void ERR_CHANOPRIVSNEEDED(Client &client, Channel &chan, int fd);
        void ERR_USERNOTINCHANNEL(Client &client, const std::string &targetNick, Channel &chan, int fd);
        void ERR_CHANNELISFULL(Client &client, Channel &chan, int fd);
        

        //rpl 461, 443, 473, 475, 331, 332, 353, 366,
        //rpl 403, 482, 441
};