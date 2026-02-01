
#include <string>

class Data
{
    public:
        Data();
        Data(int fd, std::string data, bool ready);
        ~Data();
        int fd;
        std::string data;
        bool ready;
};