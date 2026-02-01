
#include <string>
#include <vector>


class User;

class Channel
{
    public:
        Channel();
        Channel(std::string topic, std::string name, unsigned long long id);
        ~Channel();
        std::string getTopic() const;
        std::string getName() const;
        unsigned long long getId() const;
        std::vector<User *>* getUsers() const;
    private:
        std::string topic;
        std::string name;
        unsigned long long id;
        std::vector <User *> *users; 
};