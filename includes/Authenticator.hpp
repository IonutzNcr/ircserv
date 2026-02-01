#include <string>
#include "UserManager.hpp" 
#include <iostream>

class Authenticator{
    private:
        std::string password;
        void sendAuthMsg(std::string msg);
        void sendAuthErr(std::string msg);
        UserManager *refToUserManager;
        
    public:
        Authenticator();
        Authenticator(const std::string& pwd);
        bool isPassMatched(const std::string& pwd);
        void authenticate();
};


