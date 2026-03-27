

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

int Client::getFd( void ) const{
    return (Fd);
}

std::string Client::getIpAddr( void ) const{
    return (IpAddr);
}

std::string Client::getNickname( void ) const{
    return (Nickname);
}

std::string Client::getUsername( void ) const{
    return (Username);
}

std::string Client::getRealname( void ) const{
    return (Realname);
}

std::string Client::getBuffer( void ) const{
    return (Buffer);
}

bool Client::getPassAccepted( void ) const{
    return (PassAccepted);
}

bool Client::getRegistered( void ) const{
    return (Registered);
}

void Client::setFd( int fd ){
    Fd = fd;
}

void Client::setIpAddr( std::string ip ){
    IpAddr = ip;
}

void Client::setNickname( std::string nickname ){
    Nickname = nickname;
}

void Client::setUsername( std::string username ){
    Username = username;
}

void Client::setRealname( std::string realname ){
    Realname = realname;
}

void Client::setPassAccepted( bool accepted ){
    PassAccepted = accepted;
}

void Client::setRegistered( bool registered ){
    Registered = registered;
}

void Client::setLastPingTime( time_t time ){
    LastPingTime = time;
}

void Client::setLastActivityTime( time_t time ){
    LastActivityTime = time;
}

void Client::appendBuffer( std::string data ){
    Buffer += data;
}

void Client::clearBuffer( void ){
    Buffer.clear();
}

std::string Client::getOutBuffer( void ) const{
    return (OutBuffer);
}

time_t Client::getLastPingTime( void ) const{
    return (LastPingTime);
}

time_t Client::getLastActivityTime( void ) const{
    return (LastActivityTime);
}

void Client::appendOutBuffer( std::string data ){
    OutBuffer += data;
}

void Client::clearOutBuffer( void ){
    OutBuffer.clear();
}

void Client::eraseOutBuffer( size_t count ){
    OutBuffer.erase(0, count);
}
