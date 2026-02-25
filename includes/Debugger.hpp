#include <string>


class Debugger 
{
    public:
        static int *fd;
        static void createLogFile();
        static void storeLog(int type, std::string msg);
        static void destroyFD();
};