#include "../include/Server.hpp"

//? get client by fd
Client *Server::getClientByFd( int fd )
{
	if (Clients.count(fd))
		return &Clients[fd];
	return NULL;
}

//? get client by nickname
Client *Server::getClientByNickname(std::string nickname)
{
	for (std::map<int, Client>::iterator it = Clients.begin(); it != Clients.end(); ++it)
	{
		if (it->second.Nickname == nickname)
			return &it->second;
	}
	return NULL;
}

//? appends msg to the client's OutBuffer
void Server::SendReply( int fd, std::string msg )
{
	Client *client = getClientByFd(fd);
	if (client)
		client->OutBuffer += msg;
}

bool Server::NicknameInUse( std::string nickname )
{
	for (std::map<int, Client>::iterator it = Clients.begin(); it != Clients.end(); ++it)
	{
		if (it->second.Nickname == nickname)
			return true;
	}
	return false;
}

void Server::WelcomeClient( int fd )
{

	Client *client = getClientByFd(fd);
	if (!client)
		return ;

	std::string nick = client->Nickname;
	std::string prefix = ":" + std::string(SERVER_NAME) + " ";

	SendReply(fd, prefix + "001 " + nick + " :Welcome to the IRC Network, " +
		nick + "!" + client->Username + "@" + client->IpAddr + "\r\n");
	SendReply(fd, prefix + "002 " + nick + " :Your host is " + SERVER_NAME + ", running version 1.0\r\n");
	SendReply(fd, prefix + "003 " + nick + " :This server was created today\r\n");
	SendReply(fd, prefix + "004 " + nick + " " + SERVER_NAME + " 1.0 o o\r\n");
}

bool Server::isPrintable( std::string params)
{
		for (size_t i = 0; i < params.size(); i++)
		{
				if (!std::isprint(static_cast<unsigned char>(params[i])) && params[i] != '\r' && params[i] != '\n')
				{
						return (false);
				}
		}

		return (true);
}
