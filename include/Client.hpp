
#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <iostream>

class Client
{
    public:
        int             Fd;
        std::string     IpAddr;
        std::string     Nickname;
        std::string     Username;
        std::string     Realname;
        std::string     Buffer;
        std::string     OutBuffer;
        bool            PassAccepted;
        bool            Registered;
                
        Client();
        Client( int fd, std::string ip );
        ~Client();
};


#endif