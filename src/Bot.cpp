#include "../include/Server.hpp"

//? Bot sends a welcome message when someone joins a channel
//? Format: "Welcome <nickname> to channel <channelName>"
void Server::BotWelcome(std::string channelName, Client *client)
{
	if (!client)
		return ;

	// Bot sends a PRIVMSG to the channel that everyone can see
	std::string msg = ":" + std::string(BOT_NAME) + "!" + std::string(BOT_NAME)
		+ "@" + std::string(SERVER_NAME) + " PRIVMSG " + channelName
		+ " :Welcome " + client->Nickname + " to channel " + channelName + "\r\n";

	SendReply(client->Fd, msg);
}

//? ANNOUNCE command - only operators can use it
//? Usage: BOT ANNOUNCE <message>
//? The bot sends the message to ALL channels on the server
void Server::CmdBotAnnounce(std::string param, Client *client)
{
	if (!client || !client->Registered)
	{
		if (client)
			SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 451 * :You have not registered\r\n");
		return ;
	}

	// Check if the client is operator in at least one channel
	bool isOp = false;
	for (std::map<std::string, Channel>::iterator it = Channels.begin(); it != Channels.end(); ++it)
	{
		if (it->second.isOperator(client->Fd))
		{
			isOp = true;
			break ;
		}
	}

	if (!isOp)
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 481 " + client->Nickname
			+ " :Only operators can use BOT ANNOUNCE\r\n");
		return ;
	}

	if (param.empty())
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 461 " + client->Nickname
			+ " BOT :Not enough parameters. Usage: BOT ANNOUNCE <message>\r\n");
		return ;
	}

	// Send the announcement to every channel
	for (std::map<std::string, Channel>::iterator it = Channels.begin(); it != Channels.end(); ++it)
	{
		std::string msg = ":" + std::string(BOT_NAME) + "!" + std::string(BOT_NAME)
			+ "@" + std::string(SERVER_NAME) + " PRIVMSG " + it->first
			+ " :[ANNOUNCEMENT] " + param + "\r\n";

		// Send to ALL clients in the channel (senderFd = -1 means everyone)
		it->second.broadcastMessage(msg, -1);
	}

	// Confirm to the operator
	SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " NOTICE " + client->Nickname
		+ " :Announcement sent to all channels\r\n");

	std::cout << "[IRCSERV]: " << client->Nickname << " sent announcement: " << param << std::endl;
}
