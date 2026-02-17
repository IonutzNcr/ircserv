


/* class RplReply
{
    public:
        RPL_NOTOPIC(Client &cleint, Channel &chan, int fd)
        {
            std::string msg = ":server 331 " + client->GetNick() + " " + chan->getName() + " :No topic is set\r\n";
            send(fd, msg.c_str(), msg.size(), 0);
        }
        RPL_NAMREPLY();
        RPL_ENDOFNAMES();
        
} */