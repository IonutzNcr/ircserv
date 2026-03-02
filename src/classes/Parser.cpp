

#include "../../includes/Parser.hpp"
#include <iostream>
#include "../../includes/Command.hpp"
#include "../../includes/split.hpp"

Parser::Parser() {}
Parser::~Parser() {}

Command Parser::getCommand(std::string cmdline)
{
	Command cmd;
	try
	{
		std::size_t end;

		if (!cmdline.empty() && cmdline[0] == ':')
		{
			end = cmdline.find(" ");
			if (end == std::string::npos)
			{
				cmd.setCmd("");
				throw std::runtime_error("No command found after prefix");
			}
			cmd.setPrefix(cmdline.substr(1, end));
			cmdline = cmdline.substr(end + 1);
		}
		else
		{
			cmd.setPrefix("");
		}

		end = cmdline.find(" ");
		if (end == std::string::npos)
		{
			cmd.setCmd(cmdline);
			cmd.setArgs("");
			cmd.setTrailing("");
			return (cmd);
		}
		else
		{
			cmd.setCmd(cmdline.substr(0, end)); 
			cmdline = cmdline.substr(end + 1);
		}

		end = cmdline.find(":");
		if (end == std::string::npos)
		{
			cmd.setArgs(cmdline);
			cmd.setTrailing("");
			return (cmd);
		}
		else
		{
			if (end > 0)
				cmd.setArgs(cmdline.substr(0, end - 1));
			else
				cmd.setArgs("");
			cmd.setTrailing(cmdline.substr(end + 1));
		}

		std::vector<std::string> argsVector = split(cmd.getArgs(), ' ');
		cmd.setArgsVector(argsVector);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';

		cmd.setCmd("");
		cmd.setPrefix("");
		cmd.setTrailing("");
	}
	return (cmd);
}

Data &Parser::getData(int fd)
{
	std::vector<Data>::iterator dt = rawsData.begin();
	for (; dt != rawsData.end(); dt++)
	{
		if (dt->fd == fd)
			return (*dt);
	}
	
	Data data(fd, "", 0);
	rawsData.push_back(data);
	dt = rawsData.begin();
	for (; dt != rawsData.end(); dt++)
	{
		if (dt->fd == fd)
			return (*dt);
	}
	throw std::runtime_error("Something hoorible happened in getData method");
}

void Parser::concatData(std::string rawData, int fd)
{
	Data &data = getData(fd);
	data.data = std::string(data.data + rawData);
}

std::string Parser::extractcmd(int fd)
{
	Data &data = getData(fd);
	std::size_t index = data.data.find("\r\n");
	if (index == std::string::npos)
		return std::string("");
	std::string cmdline = data.data.substr(0, index);

	if (cmdline.size() > 510)
		cmdline = cmdline.substr(0, 510);

	std::string newData = data.data.substr(index + 2);
	data.data = newData;

	return (cmdline);
}

std::string Parser::getCmdtwo(int fd)
{
	Data &data = getData(fd);
	std::size_t index = data.data.find("\r\n");
	if (index == std::string::npos)
		return std::string("");
	std::string cmdline = data.data.substr(0, index);
	
	if (cmdline.size() > 510)
		cmdline = cmdline.substr(0, 510);
	return (cmdline);
}

Command Parser::get(int fd)
{
	std::string data = extractcmd(fd);
	Command cmd = getCommand(data);

	return (cmd);
}

void Parser::fill(std::string buff, int fd)
{
	concatData(buff, fd);
}

void Parser::clearData(int fd)
{
	for (std::vector<Data>::iterator it = rawsData.begin(); it != rawsData.end(); ++it)
	{
		if (it->fd == fd)
		{
			rawsData.erase(it);
			return;
		}
	}
}