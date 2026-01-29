#include "Authenticator.hpp"

Authenticator::Authenticator() : password("") {};

Authenticator::Authenticator(const std::string& pwd) : password(pwd) {};

bool Authenticator::authenticate(const std::string& pwd) const {
    return pwd == password;
}