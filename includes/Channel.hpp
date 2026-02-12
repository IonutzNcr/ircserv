#pragma once

#include <string>
#include <vector>

class Client;  // Forward declaration


class Channel
{
    public:
        Channel(std::string topic, std::string name, std::size_t id, std::string key);
        ~Channel();
        std::string getTopic() const;
        std::string getName() const;
        std::size_t getId() const;
        std::string getKey() const;

        void setTopic(std::string topic);

        std::vector<Client *> getUsers() const;
        bool addUser(Client* user);
        bool addOperator(Client* user);
        bool isOperator(Client* user) const;
        bool isUserInChannel(Client* user) const;
        void setProtectTopic(bool protect);
        bool isTopicProtected() const;
        void setInviteOnly(bool inviteOnly);
        bool isInviteOnly() const;
        std::vector<Client *> &getUserRefs();
        
       
    private:
        std::string _topic;
        std::string _name;
        std::size_t _id;
        std::string _key;
        std::vector <Client *> users;
        std::vector <Client *> operators;
        bool _protectTopic;
        bool _inviteOnly;
       /*  std::vector <Client *> invited; */
};