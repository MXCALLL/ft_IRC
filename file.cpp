#include "../include/Server.hpp"

//* JOIN function - entry point for the JOIN command
//* Splits comma-separated channels/keys, then handles each channel separately
void Server::CmdJoin(std::string param, Client *client)
{
	//* If the user sent "JOIN" with nothing after it, reject immediately.
	//* RFC requires at least one channel name.
	if (param.empty())
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 461 " + client->Nickname + " JOIN :Not enough parameters\r\n");
		std::cout << "param: " << param << " from client: " << client->Nickname << '\n';
		return;
	}

	std::istringstream			ss(param);
	std::string					channelList;	// raw channel(s), e.g. "#a,#b"
	std::string					channelName;	// one channel at a time after splitting
	std::vector<std::string>	keys;			// list of keys/passwords, one per channel
	std::string					keyList;		// raw keys, e.g. "pass1,pass2"
	std::string					singleKey;

	// First word = channel list, second word = key list (if provided)
	// Example: "JOIN #a,#b pass1,pass2" -> channelList="#a,#b", keyList="pass1,pass2"
	ss >> channelList >> keyList;

	std::istringstream	css(channelList);
	std::istringstream	kss(keyList);

	// Break the key list into separate keys, splitting on commas
	while (std::getline(kss, singleKey, ','))
		keys.push_back(singleKey);

	int i = 0;
	// Break the channel list into separate channel names, splitting on commas
	// Each channel gets matched with its key by position (index i)
	// If there's no key for this channel, it just gets an empty string
	while (std::getline(css, channelName, ','))
	{
		std::string currentKey = (i < (int)keys.size()) ? keys[i]: "";
		JoinOneChannel(channelName, currentKey, client); // handle this one channel
		++i;
	}
}

// Helper function - does the actual work of joining ONE channel
// Called once per channel when JOIN has multiple channels separated by commas
void Server::JoinOneChannel(std::string channelName, std::string key, Client *client)
{
	// Safety check - skip empty entries (can happen with trailing commas like "#a,")
	if (channelName.empty())
		return ;

	// Channel names must start with '#' or '&' and have at least one character after it.
	// Reject anything else as an invalid channel name.
	if (channelName.size() < 2 || (channelName[0] != '#' && channelName[0] != '&'))
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 403 " + client->Nickname + " " + channelName + " :No such channel\r\n");
		return;
	}

	// CASE 1: Channel doesn't exist yet - create it fresh
	if (Channels.count(channelName) == 0)
	{
		// Create the channel and register it in the server's channel map
		Channels.insert(std::make_pair(channelName, Channel(channelName)));
		// The creator becomes a member...
		Channels.at(channelName).addClient(client);
		// ...and automatically becomes the channel's first operator
		Channels.at(channelName).addOperator(client);
		std::cout << "[IRCSERV]: Channel " << channelName << " created by " << client->Nickname << "!" << std::endl;
	}
	// CASE 2: Channel already exists - try to join it
	else
	{
		// If already a member, do nothing (prevents duplicate JOIN spam)
		if (Channels.at(channelName).isClientInChannel(client->Fd))
			return ;

		// If the channel has a password set (MODE +k), the key given must match it
		if (!Channels.at(channelName).getKey().empty() && key != Channels.at(channelName).getKey())
		{
			SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 475 " + client->Nickname + " " + channelName + " :Cannot join channel (+k)\r\n");
			return ;
		}

		// If the channel has a user limit (MODE +l), reject if it's already full
		if (Channels.at(channelName).getChannelUserLimit() > 0 && Channels.at(channelName).getClientCount() >= Channels.at(channelName).getChannelUserLimit())
		{
			SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 471 " + client->Nickname + " " + channelName + " :Cannot join channel (+l)\r\n");
			return ;
		}

		// If the channel is invite-only (MODE +i), the user must have been invited first
		if (Channels.at(channelName).isInviteOnly() && !Channels.at(channelName).isInvited(client->Fd))
		{
			SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 473 " + client->Nickname + " " + channelName + " :Cannot join channel (+i)\r\n");
			return;
		}

		// All checks passed - add the user as a regular member
		Channels.at(channelName).addClient(client);

		// The invite was just "used up" - remove it so it can't be reused later
		if (Channels.at(channelName).isInviteOnly() && Channels.at(channelName).isInvited(client->Fd))
			Channels.at(channelName).removeFromInviteList(client->Fd);

		std::cout << "[IRCSERV]: " << client->Nickname << " joined existing channel " << channelName << "!" << std::endl;
	}

	// From here on, the join was successful (either created or joined) - notify everyone

	// Build the standard IRC JOIN announcement message
	std::string joinMsg = ":" + client->Nickname + "!" + client->Username + "@" + client->IpAddr + " JOIN :" + channelName + "\r\n";

	// Tell the joining client they successfully joined
	SendReply(client->Fd, joinMsg);

	// Tell everyone else already in the channel that someone new joined
	Channels.at(channelName).broadcastMessage(joinMsg, client->Fd);

	// Send the channel's topic to the new member (RFC requires this on successful join)
	std::string topic = Channels.at(channelName).getTopic();

	if (topic.empty())
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 331 " + client->Nickname + " " + channelName + " :No topic is set\r\n");
	else
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 332 " + client->Nickname + " " + channelName + " :" + topic + "\r\n");

	// Send the list of everyone currently in the channel, so the client's UI can display them
	SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 353 " + client->Nickname + " = " + channelName + " :" + Channels.at(channelName).getClientList() + "\r\n");

	// Mark the end of the name list (required by RFC after RPL_NAMREPLY)
	SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 366 " + client->Nickname + " " + channelName + " :End of /NAMES list\r\n");
}