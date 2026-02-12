
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


    std::string quitMsg = "";
    if (cmd.getTrailing() == "leaving" || cmd.getTrailing() == "")
        quitMsg = ":" + client->GetNick() + " QUIT :Client disconnected\r\n";
    else
        quitMsg = ":" + client->GetNick() + " QUIT :" + cmd.getTrailing() + "\r\n";

    client->SetMsgQuit(quitMsg); // Store the quit message in the client object
    return true;
}
