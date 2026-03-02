
#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/split.hpp"
#include "../../includes/Channel.hpp"
#include <string>
#include <cctype>
#include <set>
#include <sys/types.h>
#include <sys/socket.h>

static bool isSpecialChar(char c)
{
    return c == '[' || c == ']' || c == '\\' || c == '`'
        || c == '_' || c == '^' || c == '{' || c == '|' || c == '}';
}

bool Dispatch::parseNick(std::string line)
{
    if (line.empty())
        return (false);
    if (line.size() > 9)
        return (false);
    if (!std::isalpha(static_cast<unsigned char>(line[0])) && !isSpecialChar(line[0]))
        return (false);
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == ' ' || line[i] == ',' || line[i] == '*'
            || line[i] == '?' || line[i] == '!'
            || line[i] == '@' || line[i] == '#')
            return false;
    }
    return (true);
}


bool Dispatch::ft_nick(Command cmd, int fd)
{
    Client* client = getClientFd(fd);
    if (!client)
        return false;
    std::string choice = client->GetNick().empty() ? "*" : client->GetNick();
    std::string nick = cmd.getArgs();
    if (nick.empty() && !cmd.getTrailing().empty())
        nick = cmd.getTrailing();
    nick.erase(0, nick.find_first_not_of(" \t"));
    if (!nick.empty() && nick[0] == ':')
        nick.erase(0, 1);
    nick.erase(nick.find_last_not_of(" \t\r\n") + 1); 
    if (nick.empty()) {
        std::string msg = ":server 431 " + choice + " :No nickname given\r\n"; 
        sendAll(fd, msg);     
        return true;
    }
    if (!parseNick(nick)) {
        std::string msg = ":server 432 " + choice + " " + nick + " :Erroneus nickname\r\n";
        sendAll(fd, msg);     
        return true;
    }
    for (size_t i = 0; i < _clients.size(); ++i) {
            if (ircCaseEqual(_clients[i]->GetNick(), nick) && _clients[i] != client) {
                std::string msg = ":server 433 " + choice + " " + nick + " :Nickname is already in use\r\n";
                sendAll(fd, msg);
                return true;
            }
    }
    std::string oldNick = client->GetNick();
    client->SetNick(nick);
    if (!oldNick.empty() && client->isRegistered()) {
        std::string ret = ":" + oldNick + " NICK :" + nick + "\r\n";
        sendAll(fd, ret);
        std::set<int> notified;
        notified.insert(fd);
        for (size_t i = 0; i < _channels.size(); i++) {
            if (_channels[i]->isUserInChannel(client)) {
                std::vector<Client *> users = _channels[i]->getUsers();
                for (size_t j = 0; j < users.size(); j++) {
                    int userFd = users[j]->GetFd();
                    if (notified.find(userFd) == notified.end()) {
                        sendAll(userFd, ret);
                        notified.insert(userFd);
                    }
                }
            }
        }
    }    
    tryRegister(client);
    
    return true; 
}

std::vector<std::string> Dispatch::splitParams(std::string line) const
{
    std::vector<std::string> params;
    size_t pos = 0;

    pos = line.find(' ');
    if (pos == std::string::npos)
        return params;
    line = line.substr(pos + 1);
    while (!line.empty())
    {
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos)
            break;
        line.erase(0, start);
        if (line[0] == ':')
        {
            params.push_back(line.substr(1));
            break;
        }

        
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
    std::string choice = client->GetNick().empty() ? "*" : client->GetNick();

    if (client->isRegistered()) {
        std::string txt = ":server 462 " + choice + " :You may not reregister\r\n";
        sendAll(fd, txt);
        return true;
    }
    std::string line = cmd.getLine();
    std::vector<std::string> params = splitParams(line);
    if (params.size() < 4 || params[0].empty() || params[1].empty() || params[2].empty() || params[3].empty())
    {
        std::string msg = ":server 461 " + choice + " :Not enough parameters\r\n";
        sendAll(fd, msg);
        return true;
    }
   
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
    std::string choice = client->GetNick().empty() ? "*" : client->GetNick();
    if (client->isRegistered()) {
        std::string txt2 = ":server 462 " + choice + " :You may not reregister\r\n";
        sendAll(fd, txt2);
        return true;
    }
    std::string pass = cmd.getArgs();
    if (pass.empty() && !cmd.getTrailing().empty())
        pass = cmd.getTrailing();
    pass.erase(0, pass.find_first_not_of(" \t"));
    if (!pass.empty() && pass[0] == ':')
        pass.erase(0, 1);
    pass.erase(pass.find_last_not_of(" \t\r\n") + 1);
    if (pass.empty()) { 
        std::string msg = ":server 461 " + choice + " :PASS Not enough parameters\r\n";
        sendAll(fd, msg);
        return true;
    }
    if (pass != _password) {
        std::string txt = ":server 464 " + choice + " :Password incorrect\r\n";
        sendAll(fd, txt);
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
    while (i < target.size() && (target[i] == '@' || target[i] == '%' || target[i] == '+'))
        i++;    
    if (i < target.size() && (target[i] == '#' || target[i] == '&')) 
        return (2);
    return (1);
}


void    Dispatch::ft_privmsg(Command cmd, int fd)
{
    
    Client* client = getClientFd(fd);
    if (!client)
        return;
    std::vector<std::string>    params = splitParams(cmd.getLine());

    if (!client->isRegistered()) {
        std::string err = ":server 451 " + client->GetNick() + " :You have not registered\r\n";
        sendAll(fd, err);
        return;
    }
    if (params.size() < 2) {
        std::string err = ":server 461 " + client->GetNick() + " PRIVMSG :Not enough parameters\r\n";
        sendAll(fd, err);
        return;
    }
    if (params[0].empty()) {
        std::string err = ":server 411 " + client->GetNick() + " :No recipient given\r\n";
        sendAll(fd, err);
        return;
    }
    
    if (params[1].empty()) {
        std::string txt2 = ":server 412 " + client->GetNick() + " :No text to send\r\n";
        sendAll(fd, txt2);
        return ;
    }
    int choice = ft_choice(params[0]);
    if (choice == 1)
        ft_privmsg_client(params, fd);
    else if (choice == 2)
        ft_privmsg_channel(params, fd);
    else
        return ;

}

void    Dispatch::ft_privmsg_channel(std::vector<std::string> params, int fd)
{
    Client* client = getClientFd(fd);
    if (!client)
        return;
    std::string channelName = params[0];
    Channel* channel = getChannel(channelName);
    if (!channel) {
        std::string err = ":server 403 " + client->GetNick() + " " + channelName + " :No such channel\r\n";
        sendAll(fd, err);
        return;
    }
    if (!channel->isUserInChannel(client)) {
        std::string err = ":server 404 " + client->GetNick() + " " + channelName + " :Cannot send to channel\r\n";
        sendAll(fd, err);
        return;
    }
    std::string msg = ":" + client->GetNick() + " PRIVMSG " + channelName + " :" + params[1] + "\r\n";
    const std::vector<Client*>& members = channel->getUsers();
    for (size_t i = 0; i < members.size(); i++) {
        if (members[i]->GetFd() != fd) {
            sendAll(members[i]->GetFd(), msg);
        }
    }
    return ;
}

void    Dispatch::ft_privmsg_client(std::vector<std::string> params, int fd)
{
    Client* client = getClientFd(fd);
    if (!client) {
        return;
    }
    std::string client2 = params[0];
    int fd2 = findClient(client2);
    if (fd2 < 0) {
        std::string txt2 =  ":server 401 " + client->GetNick() + " " + client2 + " :No such nick\r\n";
        sendAll(fd, txt2);
        return ;
    }
    std::string privmsg = ":" + client->GetNick() + " PRIVMSG " + client2 + " :" + params[1] + "\r\n";
    sendAll(fd2, privmsg);
}