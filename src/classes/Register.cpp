
#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/split.hpp"
#include "../../includes/Channel.hpp"
#include <string>
#include <cctype>
#include <sys/types.h>
#include <sys/socket.h>


bool Dispatch::parseNick(std::string line)
{
    // build une fonction pour le parsing de nick (erreur 433)
    if (line.empty())
        return (false);
    if (!std::isalpha(static_cast<unsigned char>(line[0])))
        return (false);
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == ' ' || line[i] == ',' || line[i] == '*'
            || line[i] == '?' || line[i] == '!'
            || line[i] == '@' || line[i] == '#')
            return false;
    }
    return (true);
}
// a demander quand il ya un changement de nick, est ce que ca le prend en compte dans un channel
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
        std::string msg = ":serveur 431 :No nickname given\r\n";     // ERR_NONICKNAMEGIVEN (431)
        send(fd, msg.c_str(), msg.length(), 0);     
        return false;
    }
    if (!parseNick(nick)) {
        std::string msg = ":serveur 433 " + nick + ":Erroneus nickname\r\n";     // ERR_ERRONEUSNICKNAME (432)
        send(fd, msg.c_str(), msg.length(), 0);     
        return true;
    }
    for (size_t i = 0; i < _clients.size(); ++i) {  // cherhcer si le new nick est deja utiliser ou pas
            if (_clients[i]->GetNick() == nick && _clients[i] != client) {
                std::string curr = client->GetNick().empty() ? "*" : client->GetNick();  
                std::string msg = ":server 433 " + curr + " " + nick + " :Nickname is already in use\r\n"; // ERR_NICKNAMEINUSE (433)
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

std::vector<std::string> Dispatch::SplitParams(std::string line) const
{
    std::vector<std::string> params;
    size_t pos = 0;

    pos = line.find(' ');   // enlever le 1er mot(USER)
    if (pos == std::string::npos)
        return params;
    line = line.substr(pos + 1);
    while (!line.empty())   // parser les paramètres
    {
        size_t start = line.find_first_not_of(" \t");   // supprimer espaces initiaux
        if (start == std::string::npos)
            break;
        line.erase(0, start);
        if (line[0] == ':') // si ':' trouver pushback le reste
        {
            params.push_back(line.substr(1));
            break;
        }

        // paramètre normal
        size_t space = line.find(' ');
        if (space == std::string::npos)
        {
            params.push_back(line);
            break;
        }
        params.push_back(line.substr(0, space));
        line.erase(0, space + 1);
    }
    return params;
}

bool Dispatch::ft_user(Command cmd, int fd)
{   
    Client* client = getClientFd(fd);
    if (!client)
        return false;
    if (client->isRegistered()) {   // savoir si le client est enregistrer, si oui message puis on sort de la focntion
        std::string txt = ":serveur 462 :You may not reregister\r\n"; // ERR_ALREADYREGISTERED (462)
        send(fd, txt.c_str(), txt.length(), 0);
        return true;
    }
    std::string line = cmd.getLine();
    std::vector<std::string> params = SplitParams(line);
    if (params.size() < 4 || params[0].empty() || params[1].empty() || params[2].empty() || params[3].empty())
    {
        std::string msg = "USER :Not enough parameters\r\n"; // ERR_NEEDMOREPARAMS (461)
        send(fd, msg.c_str(), msg.length(), 0);
        return true;
    }
    //std::string oldUser = client->GetUser();
    client->SetUser(params[0]);
    client->SetRealName(params[3]);
    tryRegister(client);
    return true;
}

bool Dispatch::ft_pass(Command cmd, int fd)
{
    Client* client = getClientFd(fd);
    if (!client)
        return false;
    if (client->isRegistered()) {   // savoir si le client est enregistrer, si oui message puis on sort de la focntion
        std::string txt2 = ":serveur 462 * :You may not reregister\r\n";    //ERR_ALREADYREGISTERED (462)
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
        std::string msg = ":serveur 461 * PASS :Not enough parameters\r\n";    // ERR_NEEDMOREPARAMS (461)
        send(fd, msg.c_str(), msg.length(), 0);
        return true;
    }
    if (pass != _password) {    // si le pass est differend de celui des parametre => error
        std::string txt = intToString(fd) + " :Password incorrect\r\n";      // ERR_PASSWDMISMATCH (464)
        send(fd, txt.c_str(), txt.length(), 0);
        return false;
    }
    client->setAuthenticated(true);
    tryRegister(client);
   
    return true; 
}

int Dispatch::ft_choice(const std::string& target)
{
    if (target.empty())
        return 0;
    size_t i = 0;    
    while (i < target.size() && (target[i] == '@' || target[i] == '%' || target[i] == '+')) // Ignorer les préfixes de statut @, %, +
        i++;    
    if (i < target.size() && (target[i] == '#' || target[i] == '&'))    // Vérifier si c'est un channel si oui renvoie 2
        return (2);
    return (1);
}

// revoir les message d'erreur
//le cas pour channel n'est pas fait encore
void    Dispatch::ft_PRIVMSG(Command cmd, int fd)
{
    
    Client* client = getClientFd(fd);

    std::vector<std::string>    params = SplitParams(cmd.getLine());
    if (!client->isRegistered()) {
        std::string err = ":server 451 * :You have not registered\r\n";
        send(fd, err.c_str(), err.length(), 0);
        return;
    }
    if (params.size() < 2) {
        std::string err = "Error: need params " + cmd.getLine() + "\r\n";
        send(fd, err.c_str(), err.length(), 0);
        return;
    }
    if (params[1].empty()) {
        std::string err = ":server 411 * :No recipient given\r\n";
        send(fd, err.c_str(), err.length(), 0);
        return;
    }
    if (params[2].empty()) {
        std::string txt2 = ":server 412 * :No text to send\r\n";
        send(fd, txt2.c_str(), txt2.length(), 0);
        return ;
    }
    int choice = ft_choice(params[0]);
    if (choice == 1)
        ft_PRIVMSG_client(params, fd);
    else if (choice == 2)
        ft_PRIVMSG_channel(params, fd);
    else
        return ;

}

void    Dispatch::ft_PRIVMSG_channel(std::vector<std::string> params, int fd)
{
    Client* client = getClientFd(fd);
    if (!client)
        return;
    std::string channelName = params[0];
    Channel* channel = getChannel(channelName);
    if (!channel) {
        std::string err = ":server 403 * :No such channel\r\n";
        send(fd, err.c_str(), err.length(), 0);
        return;
    }
    if (!channel->isUserInChannel(client)) { // Vérifier que le client est dans le channel
        std::string err = ":server 404 * :Cannot send to channel\r\n";
        send(fd, err.c_str(), err.length(), 0);
        return;
    }
    std::string msg = ":" + client->GetNick() + " PRIVMSG " + channelName + " :" + params[1] + "\r\n";
    const std::vector<Client*>& members = channel->getUsers();
    for (size_t i = 0; i < members.size(); i++) {
        if (members[i]->GetFd() != fd) {
            send(members[i]->GetFd(), msg.c_str(), msg.length(), 0);
        }
    }
    return ;
}

void    Dispatch::ft_PRIVMSG_client(std::vector<std::string> params, int fd)
{
    Client* client = getClientFd(fd);
    if (!client) {
        return;
    }
    std::string client2 = params[0];
    int fd2 = findClient(client2);
    if (fd2 < 0) {
        std::string txt2 =  ":server 401 " + client->GetNick() + " " + client2 + " :No such nick\r\n";
        send(fd, txt2.c_str(), txt2.length(), 0);
        return ;
    }
    std::string test5 = ":" + client->GetNick() + " PRIVMSG " + client2 + " :" + params[1] + "\r\n";
    send(fd2, test5.c_str(), test5.length(), 0); //Msg envoyer au deuxieme client
}