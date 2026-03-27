
#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <iostream>
# include <ctime>

class Client
{
    private:
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

    public:
        Client();
        Client( int fd, std::string ip );
        ~Client();

        int             getFd( void ) const;
        std::string     getIpAddr( void ) const;
        std::string     getNickname( void ) const;
        std::string     getUsername( void ) const;
        std::string     getRealname( void ) const;
        std::string     getBuffer( void ) const;
        std::string     getOutBuffer( void ) const;
        bool            getPassAccepted( void ) const;
        bool            getRegistered( void ) const;
        time_t          getLastPingTime( void ) const;
        time_t          getLastActivityTime( void ) const;

        void            setFd( int fd );
        void            setIpAddr( std::string ip );
        void            setNickname( std::string nickname );
        void            setUsername( std::string username );
        void            setRealname( std::string realname );
        void            setPassAccepted( bool accepted );
        void            setRegistered( bool registered );
        void            setLastPingTime( time_t time );
        void            setLastActivityTime( time_t time );

        void            appendBuffer( std::string data );
        void            clearBuffer( void );
        void            appendOutBuffer( std::string data );
        void            clearOutBuffer( void );
        void            eraseOutBuffer( size_t count );
};


#endif