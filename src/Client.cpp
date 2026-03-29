
#include "../include/Client.hpp"


Client::Client() : Fd(-1), IpAddr(""), Nickname(""), Username(""), Realname(""),
    Buffer(""), OutBuffer(""), PassAccepted(false), Registered(false), LastPingTime(time(NULL)), LastActivityTime(time(NULL))
{

}

Client::Client( int fd, std::string ip ) : Fd(fd), IpAddr(ip), Nickname(""), Username(""),
    Realname(""), Buffer(""), OutBuffer(""), PassAccepted(false), Registered(false), LastPingTime(time(NULL)), LastActivityTime(time(NULL))
{

}

Client::~Client()
{

}
