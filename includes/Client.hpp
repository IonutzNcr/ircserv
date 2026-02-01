
#include <iostream>

class Client //-> class for client
{
	private:
		int Fd; //-> client file descriptor
		std::string IPadd; //-> client ip address

	public:
		Client();
		int GetFd(); 
		void SetFd(int fd);
		void setIpAdd(std::string ipadd); 
};