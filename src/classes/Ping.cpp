
#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/split.hpp"
#include "../../includes/Channel.hpp"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>

bool Dispatch::ft_ping(Command cmd, int fd)
{
	std::string line = cmd.getLine();
	std::string token;
	if (line.size() > 5)
		token = line.substr(5); // après "PING "
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
}