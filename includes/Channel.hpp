/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: radlouni <radlouni@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 22:28:55 by radlouni          #+#    #+#             */
/*   Updated: 2026/02/04 23:56:35 by radlouni         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

        // gerer les inviter
        bool isInvited(Client* user) const;
        
        void setInviteOnly(bool value);
        void setInviteOnly(bool value);
        
        bool isTopicRestricted() const;
        void setTopicRestricted(bool value);
        
        void invite(Client* user);
        void removeInvite(Client* user);

        void setUserLimit(std::size_t limit);
        std::size_t getUserLimit() const;

        // supprimer un utilisateur
        bool removeUser(Client* user);
        bool removeOperator(Client* user);


        // pour join
        bool isFull() const;
        bool checkKey(const std::string& key) const;
       
    private:
        bool _inviteOnly;   // +i
        bool _topicRestricted; // +t
        bool _hasKey; // +k
        std::size_t _userLimit; // +l
        std::vector<Client*> invited;
        
        std::string _topic;
        std::string _name;
        std::size_t _id;
        std::string _key;
        std::vector <Client *> users;
        std::vector <Client *> operators;
       /*  std::vector <Client *> invited; */
};