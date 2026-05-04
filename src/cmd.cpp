#include "../include/Server.hpp"

void Server::CmdPass( std::string param, Client *client )
{
	if (!client)
		return ;

	if (client->Registered)
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 462 " + client->Nickname + " :You may not reregister\r\n");
		return ;
	}

	if (param.empty())
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 461 * PASS :Not enough parameters\r\n");
		return ;
	}

	if (param != Password)
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 464 * :Password incorrect\r\n");
		return ;
	}

	client->PassAccepted = true;
	std::cout << "[IRCSERV]: fd " << client->Fd << " Password Accepted !!" << std::endl;
}

void Server::CmdNick( std::string param, Client *client )
{
	if (!client)
		return ;

	if (!client->PassAccepted)
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 451 * :You have not sent PASS\r\n");
		return ;
	}

	if (param.empty())
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 431 * :No nickname given\r\n");
		return ;
	}

	if (!std::isalpha(param[0]) && param[0] != '_' && param[0] != '-')
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 432 * " + param + " :Erroneous nickname\r\n");
		return ;
	}

	for (size_t i = 0; i < param.size(); i++)
	{
		if (!std::isalnum(param[i]) && param[i] != '_' && param[i] != '-')
		{
			SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 432 * " + param + " :Erroneous nickname\r\n");
			return ;
		}
	}

	if (NicknameInUse(param))
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 433 * " + param + " :Nickname is already in use\r\n");
		return ;
	}

	std::string oldNick = client->Nickname;
	client->Nickname = param;
	std::cout << "[IRCSERV]: fd " << client->Fd << " Nickname set to " << param << std::endl;

	if (!client->Registered && !client->Username.empty())
	{
		client->Registered = true;
		WelcomeClient(client->Fd);
		std::cout << "[IRCSERV]: fd " << client->Fd << " Registration Complete !!" << std::endl;
	}
}

void Server::CmdUser( std::string param, Client *client )
{
	if (!client)
		return ;

	if (!client->PassAccepted)
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 451 * :You have not sent PASS\r\n");
		return ;
	}

	if (client->Registered)
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 462 " + client->Nickname + " :You may not reregister\r\n");
		return ;
	}

	if (param.empty())
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 461 * USER :Not enough parameters\r\n");
		return ;
	}

	std::istringstream ss(param);
	std::string username, mode, unused, realname;

	ss >> username >> mode >> unused;

	size_t colon = param.find(':');
	if (colon != std::string::npos)
		realname = param.substr(colon + 1);
	else
		realname = username;

	if (username.empty())
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 461 * USER :Not enough parameters\r\n");
		return ;
	}

	client->Username = username;
	client->Realname = realname;
	std::cout << "[IRCSERV]: fd " << client->Fd << " Username set to " << username << std::endl;

	if (!client->Registered && !client->Nickname.empty() && !client->Username.empty())
	{
		client->Registered = true;
		WelcomeClient(client->Fd);
		std::cout << "[IRCSERV]: fd " << client->Fd << " Registration Complete !!" << std::endl;
	}
}

//? JOIN command
void Server::CmdJoin(std::string param, Client *client)
{
	//* param  => "#general password"
	//* client => client that want to join this channle (client object)

	//! pseudocode:
	/*
	?1. Parse: extract channel name (and optional key) from param    => done
	?2. Validate: does it start with '#' or '&'?                     => done
	3. Check password if channel has one (skip for now, add later)   => //todo
	?4 Create OR Join:                                               => done
	   ?- Not exists → create it, add client, make them operator     => done
	   ?- Exists → add client (check invite-only later)              => done
	?5. Broadcast the JOIN message to the channel                    => done
	?6. Send RPL_NAMREPLY (353) + RPL_ENDOFNAMES (366)               => done
	*/

	if (param.empty())
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 461 " + client->Nickname + " JOIN :Not enough parameters\r\n");
		return;
	}

	std::string	channelName;
	std::string	key;
	std::istringstream ss(param);

	ss >> channelName >> key;

	if (channelName[0] != '#' && channelName[0] != '&') //todo mr.aouanni said that we should remove the check for '&'
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
		//todo => (Note: Later, Person 3's MODE checks for passwords/limits will go here)
		//? check invite-only before joining
		if (Channels.at(channelName).getInviteOnly() && !Channels.at(channelName).isInvited(client->Nickname))
		{
			SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 473 " + client->Nickname + " " + channelName + " :Cannot join channel (+i)\r\n");
			return;
		}
		Channels.at(channelName).addClient(client);
		std::cout << "[IRCSERV]: " << client->Nickname << " joined existing channel " << channelName << "!" << std::endl;
	}

	std::string userPrefix = ":" + client->Nickname + "!" + client->Username + "@" + client->IpAddr;
	std::string joinMsg = userPrefix + " JOIN :" + channelName + "\r\n";

	SendReply(client->Fd, joinMsg);

	Channels.at(channelName).broadcastMessage(joinMsg, client->Fd);

	std::string clientList = Channels.at(channelName).getClientList();

	// 353 Format: :<server> 353 <nickname> = <channel> :<names list>
	SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 353 " + client->Nickname + " = " + channelName + " :" + clientList + "\r\n");
	
	// 366 Format: :<server> 366 <nickname> <channel> :End of /NAMES list
	SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 366 " + client->Nickname + " " + channelName + " :End of /NAMES list\r\n");
}

//? KICK Command
void Server::CmdKick(std::string param, Client *client)
{
	// TODO: Implement KICK logic here
	std::cout << "[IRCSERV]: KICK command received from fd " << client->Fd << " with param: " << param << std::endl;
}

//? INVITE Command
void Server::CmdInvite(std::string param, Client *client)
{
	//* param => Youssef #general
	//* client => client that send the invitation (client object)

	std::string	targetNick;
	std::string	channelName;
	std::istringstream ss(param);

	ss >> targetNick >> channelName;

	if (targetNick.empty() || channelName.empty())
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 461 " + client->Nickname + " INVITE :Not enough parameters\r\n");
		return;
	}

	if (channelName[0] != '#' && channelName[0] != '&') //todo mr.aouanni said that we should remove the check for '&', but i'm not sure
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 403 " + client->Nickname + " " + channelName + " :No such channel\r\n");
		return;
	}

	if (Channels.count(channelName) == 0)
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 403 " + client->Nickname + " " + channelName + " :No such channel\r\n");
		return;
	}
	
	if(!Channels.at(channelName).isClientInChannel(client->Fd))
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 442 " + client->Nickname + " " + channelName + " :You're not on that channel\r\n");
		return;
	}

	if (Channels.at(channelName).getInviteOnly() && !Channels.at(channelName).isOperator(client->Fd))
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 482 " + client->Nickname + " " + channelName + " :You're not channel operator\r\n");
		return;
	}

	if (!getClientByNickname(targetNick))
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 401 " + client->Nickname + " " + targetNick + " :No such nick/channel\r\n");
		return;
	}

	SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 341 " + client->Nickname + " " + targetNick + " " + channelName + "\r\n");
	
	SendReply(getClientByNickname(targetNick)->Fd, ":" + client->Nickname + "!" + client->Username + "@" + client->IpAddr + " INVITE " + targetNick + " :" + channelName + "\r\n");

	Channels.at(channelName).addToInviteList(targetNick);

	//! NOTE: invite can bypass the limits (if an channle has a limit of users)
}
