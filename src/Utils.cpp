#include "../include/Server.hpp"

//? get client by fd
Client *Server::getClientByFd( int fd )
{
	if (Clients.count(fd))
		return &Clients[fd];
	return NULL;
}

//? get client by nickname from server's list clients
Client *Server::getClientByNickFromServer(std::string nickname)
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
		if (!std::isprint(static_cast<unsigned char>(params[i])) && params[i] != '\r' && params[i] != '\n' && params[i] != '\x01') //! I add this check for '\x01' (the CTCP character) to handl file transfer ( must confirmed by muidbell)
			return (false);
	}
	return (true);
}

void Server::JoinOneChannel(std::string channelName, std::string key, Client *client)
{
	if (channelName.empty())
		return ;
	if (channelName.size() < 2 || (channelName[0] != '#' && channelName[0] != '&'))
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 403 " + client->Nickname + " " + channelName + " :No such channel\r\n");
		return;
	}

	if (Channels.count(channelName) == 0)
	{
		Channels.insert(std::make_pair(channelName, Channel(channelName)));
		Channels.at(channelName).addClient(client);
		Channels.at(channelName).addOperator(client);
		std::cout << "[IRCSERV]: Channel " << channelName << " created by " << client->Nickname << "!" << std::endl;
	}
	else
	{
		if (Channels.at(channelName).isClientInChannel(client->Fd))
			return ;
		//? check passwords before joining (MODE +k)
		if (!Channels.at(channelName).getKey().empty() && key != Channels.at(channelName).getKey())
		{
			SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 475 " + client->Nickname + " " + channelName + " :Cannot join channel (+k)\r\n");
			return ;
		}
		//? check limits before joining (MODE +l)
		if (Channels.at(channelName).getChannelUserLimit() > 0 && Channels.at(channelName).getClientCount() >= Channels.at(channelName).getChannelUserLimit())
		{
			SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 471 " + client->Nickname + " " + channelName + " :Cannot join channel (+l)\r\n");
			return ;
		}
		//? check invite-only before joining (MODE +i)
		if (Channels.at(channelName).getInviteOnly() && !Channels.at(channelName).isInvited(client->Fd))
		{
			SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 473 " + client->Nickname + " " + channelName + " :Cannot join channel (+i)\r\n");
			return;
		}
		Channels.at(channelName).addClient(client);

		//? remove client from invite list after he join
		if (Channels.at(channelName).getInviteOnly() && Channels.at(channelName).isInvited(client->Fd))
			Channels.at(channelName).removeFromInviteList(client->Fd);

		std::cout << "[IRCSERV]: " << client->Nickname << " joined existing channel " << channelName << "!" << std::endl;
	}

	std::string joinMsg = ":" + client->Nickname + "!" + client->Username + "@" + client->IpAddr + " JOIN :" + channelName + "\r\n";

	SendReply(client->Fd, joinMsg);

	Channels.at(channelName).broadcastMessage(joinMsg, client->Fd);

	std::string topic = Channels.at(channelName).getTopic();
	
	if (topic.empty())
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 331 " + client->Nickname + " " + channelName + " :No topic is set\r\n");
	else
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 332 " + client->Nickname + " " + channelName + " :" + topic + "\r\n");

	SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 353 " + client->Nickname + " = " + channelName + " :" + Channels.at(channelName).getClientList() + "\r\n");

	SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 366 " + client->Nickname + " " + channelName + " :End of /NAMES list\r\n");
}
