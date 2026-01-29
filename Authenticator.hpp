#include <string>

class Authenticator{
    private:
        std::string password;
    public:
        Authenticator();
        Authenticator(const std::string& pwd);
        bool authenticate(const std::string& pwd) const;
};

