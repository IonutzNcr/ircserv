#pragma once

#include <vector>
#include "Client.hpp"

#include "Channel.hpp"
class Command;

class Dispatch
{

    public:
        std::vector<Client *> &_clients;
        std::vector<Channel *> _channels;
        std::string _password;
       /*  Dispatch(); */
        Dispatch(std::string password, std::vector<Client *> &clients);
        ~Dispatch();
        void dispatch(Command cmd, int fd);

        bool ft_cap(Command cmd, int fd);
        bool ft_pass(Command cmd, int fd);
        bool ft_nick(Command cmd, int fd);
        bool ft_user(Command cmd, int fd);
        bool ft_join(Command cmd, int fd);
        bool parseNick(std::string line);
        void ft_PRIVMSG(Command cmd, int fd);
        void ft_PRIVMSG_client(Command cmd, int fd);
        void ft_PRIVMSG_channel(Command cmd, int fd);
        int	findClient(std::string nick);

        bool ft_mode(Command cmd, int fd);

        bool ft_kick(Command cmd, int fd);
        bool setMode(Channel* channel, std::string modeChanges, int fd, std::string target, std::string msg, Client* client, std::vector<std::string> tokens);
        bool ft_ping(Command cmd, int fd);
        bool ft_invite(Command cmd, int fd);
        bool ft_topic(Command cmd, int fd);
        bool ft_quit(Command cmd, int fd);
        bool ft_who(Command cmd, int fd);
        

        bool isChannelExist(std::string chanName);
        Channel*    getChannel(std::string target);
        Client *getClientFd(int fd_client);
        void tryRegister(Client* client);
};

