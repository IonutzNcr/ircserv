#include "../../includes/Authenticator.hpp"

Authenticator::Authenticator() : password("") {};

Authenticator::Authenticator(const std::string& pwd) : password(pwd) {};

bool Authenticator::isPassMatched(const std::string& pwd) {
    return (pwd == password);
}

void Authenticator::authenticate()
{
    if(isPassMatched(password))
    {

        sendAuthMsg("Welcome to server");
    }
    else
        sendAuthErr("Connection failed");
}
void Authenticator::sendAuthMsg(std::string msg)
{
    std::cout << msg << std::endl;
}

void Authenticator::sendAuthErr(std::string msg)
{
    std::cout << msg << std::endl;
}