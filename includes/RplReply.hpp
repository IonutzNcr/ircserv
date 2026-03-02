#pragma once

#include <string>

class Client;
class Channel;

class RplReply
{
    public:
        void rpl_notopic(Client &client, Channel &chan, int fd);
        void rpl_topic(Client &client, Channel &chan, int fd);
        void rpl_namreply(Client &client, Channel &chan, int fd);
        void rpl_endofnames(Client &client, Channel &chan, int fd);

        void err_needmoreparams(Client &client, const std::string &command, int fd);
        void err_useronchannel(Client &client, const std::string &targetNick, Channel &chan, int fd);
        void err_inviteonlychan(Client &client, Channel &chan, int fd);
        void err_badchannelkey(Client &client, Channel &chan, int fd);
        void err_nosuchchannel(Client &client, const std::string &channelName, int fd);
        void err_chanoprivsneeded(Client &client, Channel &chan, int fd);
        void err_usernotinchannel(Client &client, const std::string &targetNick, Channel &chan, int fd);
        void err_channelisfull(Client &client, Channel &chan, int fd);
};