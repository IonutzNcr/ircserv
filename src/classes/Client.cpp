#include "../../includes/Client.hpp"


Client::Client(int fd) : _fd(fd), _nickname(""), _username(""), _authentificated(false), _registered(false)
{
    return ;
}

