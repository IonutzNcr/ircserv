
#include <string>
#include <vector>

class Channel;

class User
{
    public: 
        std::string getNickname() const;
        std::string getUsername() const;
        std::string hostname() const;
        bool getStatus() const;
        std::vector <Channel *> *getChannels() const;
    private:    
        std::string _nickname;
        std::string  _username;
        std::string  _hostname;
        bool isOperator;
        std::vector <Channel *> *channels;
};
