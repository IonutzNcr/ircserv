
#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include "../../includes/Client.hpp"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>



Dispatch::Dispatch(std::string pass, std::vector<Client *>&clients):_password(pass), _clients(clients)
{

}

Dispatch::~Dispatch()
{

}

void Dispatch::dispatch(Command cmd, int fd)
{
    if (cmd.getCmd() == "CAP")
        ft_cap(cmd);
    if (cmd.getCmd() == "PASS")
        ft_pass(cmd, fd);
    if (cmd.getCmd() == "NICK")
        ft_nick(cmd, fd);
    if (cmd.getCmd() == "JOIN")
        ft_join(cmd, fd);
}

bool Dispatch::ft_cap(Command cmd)
{
    (void)cmd;
    return true;
}

bool Dispatch::ft_pass(Command cmd, int fd)
{
    Client* client = getClientFd(fd);
    if (!client)
        return false;
    if (client->isRegistered()) {   // savoir si le client est enregistrer, si oui message puis on sort de la focntion
        std::string txt = ":server 462 :You may not reregister\r\n";
        send(fd, txt.c_str(), txt.length(), 0);
        return false;
    }
    std::string line = cmd.getLine();
    std::string pass = line.substr(3);
    pass.erase(0, pass.find_first_not_of(" \t"));
    if (!pass.empty() && pass[0] == ':')
        pass.erase(0, 1);
    pass.erase(pass.find_last_not_of(" \t\r\n") + 1);
    if (pass.empty()) { 
        std::string msg = ":server 461 PASS :Not enough parameters\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
        return false;
    }
    if (pass != _password) {    // si le pass est differend de celui des parametre => error
        std::string txt = ":server 464 :Password incorrect\r\n";
        send(fd, txt.c_str(), txt.length(), 0);
        return false;
    }
    client->setAuthenticated(true);
    tryRegister(client);
    return true; 
}
 
 
bool Dispatch::ft_nick(Command cmd, int fd)
{
    Client* client = getClientFd(fd);
    if (!client)
        return false;
    std::string line = cmd.getLine();
    std::string nick = line.substr(4); // on stock la string apres le NICK
    nick.erase(0, nick.find_first_not_of(" \t")); // supprime les espaces ou tabulation jusqua un autre char autre que ca
    if (!nick.empty() && nick[0] == ':') // a voir pour traiter un seul ou plusieur ':'
        nick.erase(0, 1);
    nick.erase(nick.find_last_not_of(" \t\r\n") + 1); // meme chose sauf que c'est a la fin mtn
    if (nick.empty()) {     // si pas de new nick => erreur 
        std::string msg = "431 :No nickname given\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
        return false;
    }
    for (size_t i = 0; i < _clients.size(); ++i) {  // cherhcer si le new nick est deja utiliser ou pas
            if (_clients[i]->GetNick() == nick && _clients[i] != client) {
                std::string msg = "serveur 433 * " + nick + " :Nickname is already in use\r\n";
                send(fd, msg.c_str(), msg.length(), 0);
                return false;
            }
    }
    std::string oldNick = client->GetNick();
    client->SetNick(nick);
    tryRegister(client);

    std::string ret;    //  affichage et envoie
    if (!oldNick.empty())
        ret = ":" + oldNick + " NICK :" + nick + "\r\n";
    else
        ret = ":" + nick + " NICK :" + nick + "\r\n";
    send(fd, ret.c_str(), ret.length(), 0);
    return true; 
}
bool Dispatch::ft_user(Command cmd, int fd)
{
        Client* client = getClientFd(fd);
    if (!client)
        return false;
    if (client->isRegistered()) {   // savoir si le client est enregistrer, si oui message puis on sort de la focntion
        std::string txt = ":server 462 :You may not reregister\r\n";
        send(fd, txt.c_str(), txt.length(), 0);
        return false;
    }
    std::string line = cmd.getLine();
    std::string user = line.substr(4); // on stock la string apres le NICK
    user.erase(0, user.find_first_not_of(" \t")); // supprime les espaces ou tabulation jusqua un autre char autre que ca
    if (!user.empty() && user[0] == ':') // a voir pour traiter un seul ou plusieur ':'
        user.erase(0, 1);
    user.erase(user.find_last_not_of(" \t\r\n") + 1); // meme chose sauf que c'est a la fin mtn
    if (user.empty()) {     // si pas de new nick => erreur 
        std::string msg = "461 :No nickname given\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
        return false;
    }
    std::string oldUser = client->GetUser();
    client->SetUser(user);
    tryRegister(client);
    std::string ret;    //  affichage et envoie
    if (!oldUser.empty())
        ret = ":" + oldUser + " USER :" + user + "\r\n";
    else
        ret = ":" + user + " USER :" + user + "\r\n";
    send(fd, ret.c_str(), ret.length(), 0);
    return true; 
}
bool Dispatch::ft_join(Command cmd, int fd)
{
    return true;
}


Client *Dispatch::getClientFd(int fd_client)
{
    for (size_t i = 0; i < _clients.size(); ++i) {
            if (_clients[i]->GetFd() == fd_client)
                return (_clients[i]);
        }
    return (NULL);
}

void    Dispatch::tryRegister(Client* client)
{
    if (client->isRegistered())
        return ;
    if (client->isAuthenticated() && !client->GetNick().empty() && !client->GetUser().empty()) {
        client->setRegistered(true);
        //sendWelcome(client);  // fonction a buid et a renomer pour notifier que le client c'est bien enregistrer
    }
}