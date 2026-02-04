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

        bool ft_cap(Command cmd, int fd);
        bool ft_pass(Command cmd, int fd);
        bool ft_nick(Command cmd, int fd);
        bool ft_user(Command cmd, int fd);
        bool ft_join(Command cmd, int fd);

        Client *getClientFd(int fd_client);
        void tryRegister(Client* client);
};

