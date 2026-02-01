
#include <vector>
#include "User.hpp"

class Command;


class UserManager{
    // UserManager class definition
    public:
        UserManager();
        ~UserManager();
        // Other member functions and variables
        User &getUserArray();
        User &getUserByFd(int fd);
        User &getUserByNick(std::string nick);
        User *createUser(Command &cmd, int fd);
        void AddUser(const User &newUser);
    private:
        // Private member functions and variables
        std::vector<User *> users;
};