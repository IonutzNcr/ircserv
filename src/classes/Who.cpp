
#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/split.hpp"
#include "../../includes/Channel.hpp"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>

bool Dispatch::ft_who(Command cmd, int fd)
{
	Client* client = getClientFd(fd);
	if (!client)
		return false;
	if (!client->isRegistered())
		return false;
	std::string line = cmd.getLine();
	std::vector<std::string> tokens = split(line, ' ');
	std::string target = "*";
	if (tokens.size() >= 2)
		target = tokens[1];

	for (size_t j = 0; j < _clients.size(); j++) {
		Client *entry = _clients[j];
		if (target != "*" && entry->GetNick() != target)
			continue;

		std::string user = entry->GetUser().empty() ? "user" : entry->GetUser();
		std::string host = entry->GetIpAdd().empty() ? "localhost" : entry->GetIpAdd();
		std::string real = entry->GetRealName().empty() ? entry->GetNick() : entry->GetRealName();
		std::string channel = "*";
		for (size_t i = 0; i < _channels.size(); i++) {
			if (_channels[i]->isUserInChannel(entry) && _channels[i]->isUserInChannel(client)) {
				channel = _channels[i]->getName();
				break;
			}
		}

		std::string whoReply = ":server 352 " + client->GetNick() + " " + channel + " " + user + " " + host + " server " + entry->GetNick() + " H :0 " + real + "\r\n";
		send(fd, whoReply.c_str(), whoReply.length(), 0);
	}

	std::string endWho = ":server 315 " + client->GetNick() + " " + target + " :End of /WHO list\r\n";
	send(fd, endWho.c_str(), endWho.length(), 0);
	return true;
}