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
     /*    bool ft_ping(Command cmd, int fd); */
        

        bool isChannelExist(std::string chanName);
        Client *getClientFd(int fd_client);
        void tryRegister(Client* client);
};

