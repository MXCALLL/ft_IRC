#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <iostream>

//* Represents a single user connected to the IRC server.
class Client
{
	public:
		int             Fd;           //? The unique socket file descriptor for this user's connection.
		std::string     IpAddr;       //? The IP address the user connected from.
		std::string     Nickname;     //? The user's public display name (set by NICK command).
		std::string     Username;     //? The user's login username (set by USER command).
		std::string     Realname;     //? The user's real-life name (set by USER command).
		std::string     Buffer;       //? Accumulates incoming text until a full command line (\n) is received.
		std::string     OutBuffer;    //? Accumulates outgoing text waiting to be sent back to this user.
		bool            PassAccepted; //? True if the user provided the correct server password.
		bool            Registered;   //? True if the user has completed PASS, NICK, and USER.

		Client();
		Client( int fd, std::string ip );
		~Client();
};

#endif
