#ifndef SERVER_H
# define SERVER_H

# include <iostream>
# include <string>
# include <cstring>
# include <cstdlib>
# include <cerrno>
# include <map>
# include <vector>

# include <unistd.h>
# include <fcntl.h>
# include <csignal>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h> // sockaddr_in
# include <poll.h>
# include <arpa/inet.h> // init_addr()
# include "Client.hpp"

/* Server Config */
# define ADDR "0.0.0.0"
# define MAX_PENDING_CONNECTIONS 128



class Server
{
	private:
		int                                 Port;
		int                                 listenSockFd;
		static bool                         Signal;
		std::vector<Client>                 Clients;
		std::vector<struct poolfd>          Fd;


		void SetupSocket( int Port );
		void Socketreuse( int fd );
		void SetNonBlocking( int fd );

	public:
		Server();
		Server(int Port, std::string Password);
		~Server();


		void run( void );
		void stop ( void );

};
















#endif