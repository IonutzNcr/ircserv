
#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/split.hpp"
#include "../../includes/Channel.hpp"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

bool Dispatch::ft_quit(Command cmd, int fd)
{
    Client* client = getClientFd(fd);
    if (!client)
        return false;

    std::string user = client->GetUser().empty() ? "user" : client->GetUser();
    std::string host = client->GetIpAdd().empty() ? "localhost" : client->GetIpAdd();
    std::string prefix = ":" + client->GetNick() + "!" + user + "@" + host;

    std::string quitMsg = "";
    if (cmd.getTrailing() == "leaving" || cmd.getTrailing() == "")
        quitMsg = prefix + " QUIT :Client disconnected\r\n";
    else
        quitMsg = prefix + " QUIT :" + cmd.getTrailing() + "\r\n";
    std::cout << "\e[1;31m" << "Client <" << client->GetFd() << "> Disconnected" << "\e[0;37m" << std::endl;
    client->SetMsgQuit(quitMsg);
    return true;
}
