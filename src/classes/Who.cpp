
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
	std::string whoMsg = ":" + client->GetNick() + " WHO :";
	if (tokens.size() < 2) {
		std::vector<Client*> allClients = _clients;
		for (size_t i = 0; i < allClients.size(); i++) {
			int skip = 0;
			for (size_t j = 0; j < _channels.size(); j++) {
				if (_channels[j]->isUserInChannel(allClients[i]) && _channels[j]->isUserInChannel(client)) {
					skip = 1;
					break;
				}
			}
			if (skip)
				continue;
			whoMsg += allClients[i]->GetNick() + " ";
		}
		whoMsg += "\r\n";
		send(fd, whoMsg.c_str(), whoMsg.length(), 0);
	}
	else {
		std::string target = tokens[1];
		for (size_t j = 0; j < _clients.size(); j++) {
			if (_clients[j]->GetNick() == target) {
				whoMsg += _clients[j]->GetNick() + " ";
				whoMsg += _clients[j]->GetUser() + " ";
				whoMsg += _clients[j]->GetIpAdd() + " ";
				whoMsg += "H"; // to fix the status of the user (away or not)
				whoMsg += "\r\n";
				send(fd, whoMsg.c_str(), whoMsg.length(), 0);
				break;
			}
		}
	}
}