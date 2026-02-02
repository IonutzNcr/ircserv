#include <vector>



class Dispatch
{
    public:
        std::vector<User> users;
        std::vector<Channel> channels;
        std::string password;
        Dispatch();
        Dispatch(std::string password);
        ~Dispatch();
        void dispatch(Command cmd, int fd);

        void ft_cap(Command cmd);
        void ft_pass(Command cmd, int fd);
        void ft_nick(Command cmd, int fd);
        void ft_user(Command cmd, int fd);
        void ft_join(Command cmd, int fd);
};