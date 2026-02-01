
#include <vector>
#include <string>

class Channel;
class User;

class ChannelManager
{
    public:
        ChannelManager();
        ~ChannelManager();
        Channel *createChannel(std::string topic, std::string name);
        bool addUserToChannel(User *user);
    private:
        std::vector<Channel *> channels;
};