
#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <iostream>
# include <ctime>

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
        time_t          LastPingTime;
        time_t          LastActivityTime;

        Client();
        Client( int fd, std::string ip );
        ~Client();
};


#endif