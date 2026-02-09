#pragma once

#include <vector>
#include "Client.hpp"

/* #include "Channel.hpp" */
class Command;

class Dispatch
{

    public:
        std::vector<Client *> &_clients;
        /* std::vector<Channel> channels; */
        std::string _password;
       /*  Dispatch(); */
        Dispatch(std::string password, std::vector<Client *> &clients);
        ~Dispatch();
        void dispatch(Command cmd, int fd);

        bool ft_cap(Command cmd);
        bool ft_pass(Command cmd, int fd);
        bool ft_nick(Command cmd, int fd);
        bool ft_user(Command cmd, int fd);
        bool ft_join(Command cmd, int fd);
        bool parseNick(std::string line);
        void ft_PRIVMSG(Command cmd, int fd, int choice);
        void ft_PRIVMSG_client(Command cmd, int fd);
        void ft_PRIVMSG_channel(Command cmd, int fd);
        int	findClient(std::string nick);

        Client *getClientFd(int fd_client);
        void tryRegister(Client* client);
};

