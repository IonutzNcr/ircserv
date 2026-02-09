
#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include "../../includes/Client.hpp"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>



Dispatch::Dispatch(std::string pass, std::vector<Client *>&clients): _password(pass), _clients(clients)
{
    return ;
}

Dispatch::~Dispatch()
{
    return ;
}

void Dispatch::dispatch(Command cmd, int fd)
{
    if (cmd.getCmd() == "CAP")
        ft_cap(cmd);
    if (cmd.getCmd() == "PASS")
        ft_pass(cmd, fd);
    if (cmd.getCmd() == "NICK")
        ft_nick(cmd, fd);
    if (cmd.getCmd() == "USER")
        ft_user(cmd, fd);
    if (cmd.getCmd() == "JOIN")
        ft_join(cmd, fd);
    if (cmd.getCmd() == "PRIVMSG")
        ft_PRIVMSG(cmd, fd);
}
//le cas pour channel n'est pas fait encore
void    Dispatch::ft_PRIVMSG(Command cmd, int fd, int choice)
{
    if (choice == 1)
        ft_PRIVMSG_client(cmd, fd);
    else if (choice == 2)
        ft_PRIVMSG_channel(cmd, fd);
    else
        return ;

}

void    Dispatch::ft_PRIVMSG_channel(Command cmd, int fd)
{
    Client* client = getClientFd(fd);
    if (!client)
        return;
    if (!client->isRegistered()) {
        std::string err = "You are not registered\r\n";
        send(fd, err.c_str(), err.length(), 0);
        return;
    }
    if (cmd.getArgs().empty()) {
        std::string err = "Error: no recipient\r\n";
        send(fd, err.c_str(), err.length(), 0);
        return;
    }
    std::string channelName = cmd.getArgs();
    if (cmd.getTrailing().empty()) {
        std::string err = "Error: no text to send\r\n";
        send(fd, err.c_str(), err.length(), 0);
        return;
    }

    Channel* channel = getChannel(channelName);
    if (!channel) {
        std::string err = "Error: no such channel\r\n";
        send(fd, err.c_str(), err.length(), 0);
        return;
    }

    if (!channel->hasClient(client)) { // Vérifier que le client est dans le channel
        std::string err = "Error: you're not on that channel\r\n";
        send(fd, err.c_str(), err.length(), 0);
        return;
    }
    std::string msg = ":" + client->GetNick() + " PRIVMSG " + channelName
        + " :" + cmd.getTrailing() + "\r\n";
    const std::vector<Client*>& members = channel->getClients();
    for (size_t i = 0; i < members.size(); i++) {
        if (members[i]->getFd() != fd) {
            send(members[i]->getFd(), msg.c_str(), msg.length(), 0);
        }
    }
    return ;
}

void    Dispatch::ft_PRIVMSG_client(Command cmd, int fd)
{
    Client* client = getClientFd(fd);
    if (cmd.getArgs().empty()) {
        std::string txt2 = "Error: no destinataire\r\n";
        send(fd, txt2.c_str(), txt2.length(), 0);
        return ;
    }
    std::string client2 = cmd.getArgs();
    if (!client)
        return ;
    if (!client->isRegistered()) {
        std::string txt2 = "You are not register\r\n";
        send(fd, txt2.c_str(), txt2.length(), 0);
        return ;
    }
    if (cmd.getTrailing().empty()) {
        std::string txt2 = "Error: texte missing\r\n";
        send(fd, txt2.c_str(), txt2.length(), 0);
        return ;
    }
    // mettre une protction pour savoir si il ya bien un message
    int fd2 = findClient(client2);
    if (fd2 < 0) {
        std::string txt2 = "Error: no such nick\r\n";
        send(fd, txt2.c_str(), txt2.length(), 0);
        return ;
    }
    std::string sendText = ":" + client->GetNick() + " PRIVMSG " + client2 + " :" + cmd.getTrailing() + "\r\n";
    send(fd2, sendText.c_str(), sendText.length(), 0); //Msg envoyer au deuxieme client

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
        std::string txt2 = fd + " :You may not reregister\r\n";    //ERR_ALREADYREGISTERED (462)
        send(fd, txt2.c_str(), txt2.length(), 0);
        return true;
    }
  
    std::string line = cmd.getLine();
    
    std::string pass = line.substr(4);
   
    pass.erase(0, pass.find_first_not_of(" \t"));
    if (!pass.empty() && pass[0] == ':')
        pass.erase(0, 1);
    pass.erase(pass.find_last_not_of(" \t\r\n") + 1);
    if (pass.empty()) { 
        std::string msg = fd + " PASS :Not enough parameters\r\n";    // ERR_NEEDMOREPARAMS (461)
        send(fd, msg.c_str(), msg.length(), 0);
        return true;
    }
    if (pass != _password) {    // si le pass est differend de celui des parametre => error
        std::string txt = fd + " :Password incorrect\r\n";      // ERR_PASSWDMISMATCH (464)
        send(fd, txt.c_str(), txt.length(), 0);
        return true;
    }
   
    client->setAuthenticated(true);
    tryRegister(client);
   
    return true; 
}
 


bool Dispatch::parseNick(std::string line)
{
    // build une fonction pour le parsing de nick (erreur 433)






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
        std::string msg = fd + " :No nickname given\r\n";     // ERR_NONICKNAMEGIVEN (431)
        send(fd, msg.c_str(), msg.length(), 0);     
        return false;
    }
    for (size_t i = 0; i < _clients.size(); ++i) {  // cherhcer si le new nick est deja utiliser ou pas
            if (_clients[i]->GetNick() == nick && _clients[i] != client) {
                std::string msg = fd + " " + nick + " :Nickname is already in use\r\n"; // ERR_NICKNAMEINUSE (433)
                send(fd, msg.c_str(), msg.length(), 0);
                return true;
            }
    }
    std::string oldNick = client->GetNick();
    client->SetNick(nick);
    if (!oldNick.empty() && client->isRegistered()) {       // si le client est deja enregistrer et qu'il modifie le nick
        std::string ret = ":" + oldNick + " NICK :" + nick + "\r\n";
        send(fd, ret.c_str(), ret.length(), 0);
    }    
    tryRegister(client);
    return true; 
}

bool Dispatch::ft_user(Command cmd, int fd)
{   
    Client* client = getClientFd(fd);
    if (!client)
        return false;
    if (client->isRegistered()) {   // savoir si le client est enregistrer, si oui message puis on sort de la focntion
        std::string txt = fd + " :You may not reregister\r\n"; // ERR_ALREADYREGISTERED (462)
        send(fd, txt.c_str(), txt.length(), 0);
        return true;
    }
    std::string line = cmd.getLine();
    std::string user = line.substr(4); // on stock la string apres le NICK
    user.erase(0, user.find_first_not_of(" \t")); // supprime les espaces ou tabulation jusqua un autre char autre que ca
    if (!user.empty() && user[0] == ':') // a voir pour traiter un seul ou plusieur ':'
        user.erase(0, 1);
    user.erase(user.find_last_not_of(" \t\r\n") + 1); // meme chose sauf que c'est a la fin mtn
    if (user.empty()) {     // si pas de new nick => erreur 
        std::string msg = fd + " USER :Not enough parameters\r\n"; // ERR_NEEDMOREPARAMS (461)
        send(fd, msg.c_str(), msg.length(), 0);
        return true;
    }
    std::string oldUser = client->GetUser();
    client->SetUser(user);
    tryRegister(client);
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
        std::string msg = ":server 001 " + client->GetNick() + " :Welcome to the IRC server!\r\n";
        send(client->GetFd(), msg.c_str(), msg.length(), 0);
    }
    if (!client->isAuthenticated() && !client->GetNick().empty() && !client->GetUser().empty()) {
        std::string msg2 = "464 * :Password incorrect\r\n";
        send(client->GetFd(), msg2.c_str(), msg2.length(), 0);
        // a voir si on deconnecte le client ou pas
    }
}

int	Dispatch::findClient(std::string nick)
{
	for(size_t i = 0; i < _clients.size(); i++){
		if (_clients[i]->GetNick() == nick) {
                return (_clients[i]->GetFd());
            }
	}
	return (-1);
}