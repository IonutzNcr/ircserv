#pragma once

#include <iostream>
#include <string>



class Client //-> class for client
{
	    private :
        int _fd;
        std::string _IPadd;
        std::string _nickname;
        std::string _realname;
        std::string _username;
        bool        _authentificated;
        bool        _registered;
    public :
        Client(int fd);
        ~Client() {return ;}
        
    // --- Fonction ---
    int GetFd() {return _fd;}
    void SetFd(int FD) {_fd = FD;}

    std::string GetNick() const {return _nickname;}
    std::string GetUser() const {return _username;}
    std::string GetRealName() const {return _realname;}
    void        SetNick(const std::string &nick) {_nickname = nick;}
    void        SetUser(const std::string &user) {_username = user;}
    void        SetRealName(const std::string &realname) {_realname = realname;}
    void        SetIpAdd(std::string add) {_IPadd = add;}
    void        setRegistered(bool etat) {_registered = etat;}
    bool        isRegistered() const {return _registered;}
    void        setAuthenticated(bool etat) {_authentificated = etat;}
    bool        isAuthenticated() const {return _authentificated;}
};