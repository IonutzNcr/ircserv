
#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/split.hpp"
#include "../../includes/Channel.hpp"
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
    /* if (cmd.getCmd() == "PING")
        ft_ping(cmd, fd);  */
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
        std::string msg = "CAP * NAK :\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
    }
    else if (line.find("END") != std::string::npos) {
        // nothing needed, client ends negotiation
    }
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
    // std::string mssg = "ft_Nick set a true " + nick + "\r\n";
    // send(fd, mssg.c_str(), mssg.size(), 0);
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
    // std::string mssg = "ft_User set a true " + user + "\r\n";
    // send(fd, mssg.c_str(), mssg.size(), 0);
    tryRegister(client);
    std::string ret;    //  affichage et envoie
    if (!oldUser.empty())
        ret = ":" + oldUser + " USER :" + user + "\r\n";
    else
        ret = ":" + user + " USER :" + user + "\r\n";
    send(fd, ret.c_str(), ret.length(), 0);
    return true; 
}


//TODO::RPL_TOPIC
//TODO::RPL_NAMREPLY
bool Dispatch::ft_join(Command cmd, int fd)
{
    Client* client = getClientFd(fd);
    if (!client)
        return false;
     if (!client->isRegistered()) // si le client n'es pas register just return false
        return false;
    std::string line = cmd.getLine();
    std::string channelName = line.substr(5); // on stock la string apres le JOIN
    std::vector<std::string> chanXkeys = split(channelName, ' ');
    if (chanXkeys.empty())
    {
        std::string msg = ":server 461 JOIN :Not enough parameters\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
        return false;
    }
    std::string chanNames = chanXkeys[0];
    bool isKey = false;
    if (chanXkeys.size() > 1)
        isKey = true;
    std::string key;
    if (isKey)
        key = chanXkeys[1];
    std::vector<std::string> chanNamesSplit = split(chanNames, ',');
    std::vector<std::string>keysSplit;
    if (isKey)
        keysSplit = split(key, ',');
    for (size_t i = 0; i < chanNamesSplit.size(); i++)
    {
        std::string chanName = chanNamesSplit[i];
        std::string chanKey;
        if (isKey && i < keysSplit.size())
            chanKey = keysSplit[i];
        else
            chanKey = "";
        //creation channel si n'existe pas et ajout user au channel
        Channel *newChan = new Channel("", chanName, i, chanKey); // to fix the id is not really unique
        if (!isChannelExist(chanName)) // TODO:: big problem why ?
        {
            //print les channels existants
            std::cout << "Existing channels: ";
            for (size_t j = 0; j < _channels.size(); j++)
            {
                std::cout << _channels[j]->getName() << " ";
            }
            std::cout << std::endl;
            
            _channels.push_back(newChan);
            newChan->addUser(client);
            newChan->addOperator(client);
            std::string msg = ":server JOIN " + chanName + "\r\n";
            send(fd, msg.c_str(), msg.length(), 0);
            msg = "I created the channel " + chanName + "\r\n";
            send(fd, msg.c_str(), msg.length(), 0);
            //RPL_NOTOPIC
            msg = "331 " + newChan->getName() + " :No topic is set" + "\r\n";
            send(fd, msg.c_str(), msg.size(), 0);
            //RPL_NAMRPLY
            msg = "353 " + newChan->getName() + " :" + client->GetNick() + "\r\n";
            send(fd, msg.c_str(), msg.size(), 0);
        }else
        {
            for (size_t j = 0; j < _channels.size(); j++)
            {
                if (_channels[j]->getName() == chanName)
                {
                  std::string  msg = "I found the channel " + chanName + "\r\n";
                    send(fd, msg.c_str(), msg.length(), 0);
                    if (_channels[j]->isUserInChannel(client))
                    {
                        msg = "User is already in the channel " + chanName + "\r\n";
                        send(fd, msg.c_str(), msg.length(), 0);
                        continue; // a voir que faire si deja dans le channel et mauvais key
                    }
                    if (_channels[j]->getKey() != chanKey)
                    {
                        std::string errMsg = ":server 475 " + client->GetNick() + " " + chanName + " :Cannot join channel (+k)\r\n";
                        send(fd, errMsg.c_str(), errMsg.length(), 0);
                        continue;
                    }

                    msg = ":server JOIN " + chanName + "\r\n";
                    send(fd, msg.c_str(), msg.length(), 0);
                    msg = "User " + client->GetNick() + " joined the channel " + chanName + "\r\n";
                    send(fd, msg.c_str(), msg.length(), 0);
                    _channels[j]->addUser(client);
                    if(_channels[j]->getTopic().empty())
                    {
                        msg = "331 " + _channels[j]->getName() + " :No topic is set" + "\r\n";
                        send(fd, msg.c_str(), msg.size(), 0);
                    } else
                    {
                        msg = "332 " + _channels[j]->getName() + " :" + _channels[j]->getTopic() + "\r\n"; 
                        send(fd, msg.c_str(), msg.size(), 0);
                    }
                    //TODO::a specifier les operateurs avec @ dans la liste
                    msg = "353 " + newChan->getName() + " :";
                    std::vector<Client *> channelUsers = _channels[j]->getUsers();
                    for (std::size_t i = 0; i < channelUsers.size();i++)
                    {
                        msg +=channelUsers[i]->GetNick();
                    }
                    msg += "\r\n";
                    send(fd, msg.c_str(), msg.size(), 0);
                }
            }
        }        
    }
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
        std::string msg = ":server 001 " + client->GetNick() + " :Welcome to the IRC server!\r\n";
        send(client->GetFd(), msg.c_str(), msg.length(), 0);
    }
    if (!client->isAuthenticated() && !client->GetNick().empty() && !client->GetUser().empty()) {
        
    }
}

bool Dispatch::isChannelExist(std::string chanName)
{
    for (size_t i = 0; i < _channels.size(); i++)
    {
        if (_channels[i]->getName() == chanName)
            return true;
    }
    return false;
}

/* bool Dispatch::ft_ping(Command cmd, int fd)
{
    std::string line = cmd.getLine();
    std::string token;
    if (line.size() > 5)
        token = line.substr(5);  // après "PING "
    else
        token = "server";
    
    // Trim whitespace
    size_t start = token.find_first_not_of(" \t\r\n");
    size_t end = token.find_last_not_of(" \t\r\n");
    if (start != std::string::npos && end != std::string::npos)
        token = token.substr(start, end - start + 1);
    
    std::string reply = ":server PONG server :" + token + "\r\n";
    send(fd, reply.c_str(), reply.length(), 0);
    return true;
} */