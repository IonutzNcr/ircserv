
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
        ft_cap(cmd, fd);
    if (cmd.getCmd() == "PASS")
        ft_pass(cmd, fd);
    if (cmd.getCmd() == "NICK")
        ft_nick(cmd, fd);
    if (cmd.getCmd() == "USER")
        ft_user(cmd, fd);
    if (cmd.getCmd() == "JOIN")
        ft_join(cmd, fd);
}

bool Dispatch::ft_cap(Command cmd, int fd)
{
    std::string line = cmd.getLine();
    // respond minimally so clients finish capability negotiation
    if (line.find("LS") != std::string::npos) {
        std::string msg = "CAP * LS :\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
    }
    else if (line.find("REQ") != std::string::npos) {
        // we don't support any optional caps: NAK
        std::string msg = "CAP * NAK :\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
    }
    else if (line.find("END") != std::string::npos) {
        // client finished capability negotiation — reply with minimal ACK (no caps accepted)
        std::string msg = "CAP * ACK :\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
    }
    return true;
}

bool Dispatch::ft_pass(Command cmd, int fd)
{
    //send(fd, "PASS command received\r\n", 23, 0); // debug
    Client* client = getClientFd(fd);
   
    if (!client)
        return false;
    if (client->isRegistered()) {   // savoir si le client est enregistrer, si oui message puis on sort de la focntion
        std::string txt = ":server 462 * :You may not reregister\r\n";
        send(fd, txt.c_str(), txt.length(), 0);
        return false;
    }
  
    std::string line = cmd.getLine();
    if (line.size() <= 4) {
        std::string msg = ":server 461 PASS :Not enough parameters\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
        return false;
    }

    std::string pass = line.substr(4);
   
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
    //send(fd, "NICK command received\r\n", 23, 0); // debug
    Client* client = getClientFd(fd);
    if (!client)
        return false;
    std::string line = cmd.getLine();
    if (line.size() <= 4) {
        std::string msg = ":server 431 * :No nickname given\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
        return false;
    }
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
                std::string msg = ":server 433 * " + nick + " :Nickname is already in use\r\n";
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
        std::string txt = ":server 462 * :You may not reregister\r\n";
        send(fd, txt.c_str(), txt.length(), 0);
        return false;
    }
    std::string line = cmd.getLine();
    if (line.size() <= 4) {
        std::string msg = ":server 461 USER :Not enough parameters\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
        return false;
    }
    std::string user = line.substr(4); // on stock la string apres le NICK
    user.erase(0, user.find_first_not_of(" \t")); // supprime les espaces ou tabulation jusqua un autre char autre que ca
    if (!user.empty() && user[0] == ':') // a voir pour traiter un seul ou plusieur ':'
        user.erase(0, 1);
    user.erase(user.find_last_not_of(" \t\r\n") + 1); // meme chose sauf que c'est a la fin mtn
    if (user.empty()) {     // si pas de new nick => erreur 
        std::string msg = ":server 461 USER :Not enough parameters\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
        return false;
    }
    std::string oldUser = client->GetUser();
    client->SetUser(user);
    tryRegister(client);
    // Do not send a raw USER echo. Welcome numerics are sent from tryRegister().
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
        std::string nick = client->GetNick();
        std::string server = "server";
        std::string msg1 = ":" + server + " 001 " + nick + " :Welcome to the IRC Network, " + nick + "\r\n";
        std::string msg2 = ":" + server + " 002 " + nick + " :Your host is " + server + ", running version 0.1\r\n";
        std::string msg3 = ":" + server + " 003 " + nick + " :This server was created for testing\r\n";
        std::string msg4 = ":" + server + " 004 " + nick + " " + server + " 0.1 oO O\r\n";
        send(client->GetFd(), msg1.c_str(), msg1.length(), 0);
        send(client->GetFd(), msg2.c_str(), msg2.length(), 0);
        send(client->GetFd(), msg3.c_str(), msg3.length(), 0);
        send(client->GetFd(), msg4.c_str(), msg4.length(), 0);
    }
}