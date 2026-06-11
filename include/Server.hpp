#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>
# include <cstring>
# include <cstdlib>
# include <vector>
# include <map>
# include <sstream>
# include <unistd.h>
# include <fcntl.h>
# include <csignal>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <poll.h>
# include <arpa/inet.h>
# include "Client.hpp"
# include "Channel.hpp"
# include "Bot.hpp"

//* Server Config *//
# define ADDR "0.0.0.0"
# define BUFFER_SIZE 1024
# define SERVER_NAME "ircserv"
# define MAX_PORT 65535
# define MAX_SYS_PORT 1023

//* Server class
class Server
{
	private:
		int                                 Port;
		int                                 listenSockFd;
		std::string                         Password;
		static bool                         Signal;
		std::map<int, Client>               Clients;
		std::map<std::string, Channel>		Channels;
		std::vector<struct pollfd>          Fd;

		void SetupSocket( int Port );
		void SetNonBlocking( int fd );

		void AcceptClient( void );
		void ReceiveData( int fd );
        void SendData( int fd );
        void DisconnectClient( int fd );
        void HandleCommand( std::string cmd, int fd );

        //* Auth Commands *//
        void CmdPass( std::string param, Client *client );
        void CmdNick( std::string param, Client *client );
        void CmdUser( std::string param, Client *client );

		//* Channel Commands *//
		void CmdJoin( std::string param, Client *client );
		void JoinOneChannel(std::string channelName, std::string key, Client *client);
        void CmdKick( std::string param, Client *client );
        void CmdInvite( std::string param, Client *client );

		void CmdMode( std::string param, Client *client);
		void CmdTopic( std::string param, Client *client);
		void CmdPrivmsg( std::string param, Client *client);

		Client *getClientByFd( int fd );
		Client *getClientByNickFromServer(std::string nickname);
		void SendReply( int fd, std::string msg );
		void WelcomeClient( int fd );

		bool NicknameInUse( std::string nickname );
		bool isPrintable( std::string Params);

	public:
		Server();
		Server(int Port, std::string Password);
		~Server();

		//* Server Actions *//
		void run( Bot &IRCBot );
		void stop ( void );
		static void SignalHandler( int signum );
};

#endif
