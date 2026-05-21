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
		client->PassAccepted = false; //? I add this line here to fix the bug of the last password sent is used for verification, but still not confermed by (muidbell)
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

//? Helper fun of JOIN Cmd
void Server::JoinOneChannel(std::string channelName, std::string key, Client *client)
{
	(void)key;
	if (channelName.empty())
		return ;
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
		//? check passwords before joining (MODE +k)
		if (!Channels.at(channelName).getKey().empty() && key != Channels.at(channelName).getKey())
		{
			SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 475 " + client->Nickname + " " + channelName + " :Cannot join channel (+k)\r\n");
			return ;
		}
		//? check limits before joining (MODE +l)
		if (Channels.at(channelName).getChannelUserLimit() > 0 && Channels.at(channelName).getClientCount() >= Channels.at(channelName).getChannelUserLimit()) //?why not just use the seconde conditions (check if clients count is greater or equal to userlimit)
		{
			SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 471 " + client->Nickname + " " + channelName + " :Cannot join channel (+l)\r\n");
			return ;
		}
		//? check invite-only before joining
		if (Channels.at(channelName).getInviteOnly() && !Channels.at(channelName).isInvited(client->Nickname))
		{
			SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 473 " + client->Nickname + " " + channelName + " :Cannot join channel (+i)\r\n");
			return;
		}
		Channels.at(channelName).addClient(client);
		std::cout << "[IRCSERV]: " << client->Nickname << " joined existing channel " << channelName << "!" << std::endl;
	}

	std::string joinMsg = ":" + client->Nickname + "!" + client->Username + "@" + client->IpAddr + " JOIN :" + channelName + "\r\n";

	SendReply(client->Fd, joinMsg);

	Channels.at(channelName).broadcastMessage(joinMsg, client->Fd);

	std::string clientList = Channels.at(channelName).getClientList();

	// 353 Format: :<server> 353 <nickname> = <channel> :<names list>
	SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 353 " + client->Nickname + " = " + channelName + " :" + clientList + "\r\n");

	// 366 Format: :<server> 366 <nickname> <channel> :End of /NAMES list
	SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 366 " + client->Nickname + " " + channelName + " :End of /NAMES list\r\n");

}

//? JOIN command
void Server::CmdJoin(std::string param, Client *client)
{
	if (param.empty())
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 461 " + client->Nickname + " JOIN :Not enough parameters\r\n");
		return;
	}

	std::istringstream			ss(param);
	std::string					channelList;
	std::string					channelName;
	std::vector<std::string>	keys;
	std::string					keyList;
	std::string					singleKey;

	ss >> channelList >> keyList;

	std::istringstream	css(channelList);
	std::istringstream	kss(keyList);

	while (std::getline(kss, singleKey, ','))
		keys.push_back(singleKey);

	int i = 0;
	while (std::getline(css, channelName, ','))
	{
		std::string currentKey = (i < (int)keys.size()) ? keys[i]: "";
		JoinOneChannel(channelName, currentKey, client);
		++i;
	}
}

//? KICK Command
void Server::CmdKick( std::string param, Client *client )
{
	std::istringstream	ss(param);
	std::string			channelName;
	std::string			targetNick;
	std::string			reason;
	size_t				colon;
	Client				*target;

	ss >> channelName >> targetNick;

	colon = param.find(':');
	if (colon != std::string::npos)
		reason = param.substr(colon + 1);
	else
		reason = client->Nickname;

	if (channelName.empty() || targetNick.empty())
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 461 " + client->Nickname + " KICK :Not enough parameters\r\n");
		return ;
	}
	if (!Channels.count(channelName))
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 403 " + client->Nickname + " " + channelName + " :No such channel\r\n");
		return ;
	}

	Channel &channel = Channels.at(channelName);

	if(!channel.isClientInChannel(client->Fd))
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 442 " + client->Nickname + " " + channelName + " :You're not on that channel\r\n");
		return ;
	}
	if(!channel.isOperator(client->Fd))
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 482 " + client->Nickname + " " + channelName + " :You're not channel operator\r\n");
		return ;
	}
	if (!getClientByNickFromServer(targetNick))
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 401 " + client->Nickname + " " + targetNick + " :No such nick/channel\r\n");
		return ;
	}

	target = channel.getClientByNickFromChannel(targetNick);

	if (!target)
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 441 " + client->Nickname + " " + targetNick + " " + channelName + " :They aren't on that channel\r\n");
		return ;
	}
	channel.broadcastMessage(":" + client->Nickname + "!" + client->Username + "@" + client->IpAddr + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n", -1);

	channel.removeClient(target->Fd);

	if (channel.isEmpty())
		Channels.erase(channelName);

	std::cout << "[IRCSERV]: " << client->Nickname << " kicked " << targetNick << " from " << channelName << " (" << reason << ")" << std::endl;
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

	if (!Channels.count(channelName))
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

	if (!getClientByNickFromServer(targetNick))
	{
		SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 401 " + client->Nickname + " " + targetNick + " :No such nick/channel\r\n");
		return;
	}

	SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 341 " + client->Nickname + " " + targetNick + " " + channelName + "\r\n");

	SendReply(getClientByNickFromServer(targetNick)->Fd, ":" + client->Nickname + "!" + client->Username + "@" + client->IpAddr + " INVITE " + targetNick + " :" + channelName + "\r\n");

	Channels.at(channelName).addToInviteList(targetNick);

	//! NOTE: invite can bypass the limits (if an channle has a limit of users), check that later
}

// ‹commands/mode-topic-privmsg›

void    Server::CmdTopic( std::string param, Client *client)
{
    if (!client->Registered)
    {
        SendReply(client->Fd, ":" + std::string(SERVER_NAME)
            + " 451 * :You have not registered\r\n");
        return ;
    }
    // step 1 : parse chanel name and opt new topic
    // params comes in 2 forms :
    // "#general"                   -> just viewing
    // "#general    :new topic"     ->changing topic

    std::string channelName;
    std::string newTopic;
    bool        changingTopic = false;

    std::istringstream ss(param);
    ss >> channelName; //first name = channel name

    size_t colonPos = param.find(':');
    if (colonPos != std::string::npos)
    {
        newTopic = param.substr(colonPos + 1); // everthing after ':'
        changingTopic = true;
    }
    // step 2 : validation

    // no channel name giver
    if (channelName.empty())
    {
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 461 "
            + client->Nickname + " TOPIC :Not enough parameters\r\n");
        return ;
    }

    // channel doest exist
    if (Channels.find(channelName) == Channels.end())
    {
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 403 "
            + client->Nickname + " " + channelName + " :No such channel\r\n");
        return ;
    }

    Channel &channel = Channels.at(channelName); // refrence to the actual channel

    // client not in channel
    if (!channel.isClientInChannel(client->Fd))
    {
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 442 "
            + client->Nickname + " " + channelName + " :You are not in that channel\r\n");
        return ;
    }

    // step3: view topic or (NO topic provided)
    if (!changingTopic)
    {
        std::string topic = channel.getTopic();
        if (topic.empty())
        {
            // no topic
            SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 331 "
                + client->Nickname + " " + channelName + " :No topic is set\r\n");
        }
        else
        {
            // here is the topicc
            SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 332 "
                + client->Nickname + " " + channelName + " :" + topic + "\r\n");
        }
        return ;
    }

    // step 4: change topic
    // if mode +t is ON, only operator can change topic
    if (channel.isTopicRestricted() && !channel.isOperator(client->Fd))
    {
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 482 "
            + client->Nickname + " " + channelName + " :You are not channel operator\r\n");
        return ;
    }

    // set new topicc
    channel.setTopic(newTopic);

    // broadcast to evryojne includinhg the sender
    // format: :nick!user@host TOPIC #channel :new topic
    std::string broadcast = ":" + client->Nickname + "!" + client->Username
        + "@" + client->IpAddr + " TOPIC " + channelName + " :" + newTopic + "\r\n";

    channel.broadcastMessage(broadcast, -1);

    std::cout << "[IRCSERV]: TOPIC command received from fd " << client->Fd << " with param: " << param << std::endl;
}

void    Server::CmdPrivmsg( std::string param, Client *client)
{
    if (!client->Registered)
    {
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 451 * :You have not registered\r\n");
        return ;
    }

    std::string target;
    std::string message;

    std::istringstream ss(param);
    ss >> target;

    size_t colonPos = param.find(':');
    if (colonPos != std::string::npos)
        message = param.substr(colonPos + 1);

    if (target.empty())
    {
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 411 "
            + client->Nickname + " :No recipient given (PRIVMSG)\r\n");
        return ;
    }

    if (message.empty())
    {
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 412 "
            + client->Nickname + " :No text to send\r\n");
        return ;
    }

    std::string prefix = ":" + client->Nickname + "!" + client->Username + "@" + client->IpAddr;

    // target is a channel
    if (target[0] == '#' || target[0] == '&')
    {
        if (Channels.find(target) == Channels.end())
        {
            SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 403 "
                + client->Nickname + " " + target + " :No such channel\r\n");
            return ;
        }

        Channel &channel = Channels.at(target);

        if (!channel.isClientInChannel(client->Fd))
        {
            SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 404 "
                + client->Nickname + " " + target + " :Cannot send to channel\r\n");
            return ;
        }

        channel.broadcastMessage(prefix + " PRIVMSG " + target + " :" + message + "\r\n", client->Fd);
    }
    else
    {
        // target is a nickname
        Client *targetClient = getClientByNickFromServer(target);

        if (!targetClient)
        {
            SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 401 "
                + client->Nickname + " " + target + " :No such nick/channel\r\n");
            return ;
        }

        targetClient->OutBuffer += prefix + " PRIVMSG " + target + " :" + message + "\r\n";
    }
}


void    Server::CmdMode( std::string param, Client *client)
{
    if (!client->Registered)
    {
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 451 * :You have not registered\r\n");
        return ;
    }

    std::istringstream ss(param);
    std::string channelName, modeStr;
    ss >> channelName >> modeStr;

    if (channelName.empty())
    {
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 461 "
            + client->Nickname + " MODE :Not enough parameters\r\n");
        return ;
    }

    if (Channels.find(channelName) == Channels.end())
    {
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 403 "
            + client->Nickname + " " + channelName + " :No such channel\r\n");
        return ;
    }

    Channel &channel = Channels.at(channelName);

    if (!channel.isClientInChannel(client->Fd))
    {
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 442 "
            + client->Nickname + " " + channelName + " :You're not on that channel\r\n");
        return ;
    }

    // view current modes: MODE #channel
    if (modeStr.empty())
    {
        std::string modes = "+";
        std::string modeParams;
        if (channel.isInviteOnly()) modes += "i";
        if (channel.isTopicRestricted()) modes += "t";
        if (!channel.getKey().empty()) { modes += "k"; modeParams += " " + channel.getKey(); }
        if (channel.getUserLimit() > 0)
        {
            std::ostringstream oss;
            oss << channel.getUserLimit();
            modes += "l";
            modeParams += " " + oss.str();
        }
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 324 "
            + client->Nickname + " " + channelName + " " + modes + modeParams + "\r\n");
        return ;
    }

    // changing modes requires operator
    if (!channel.isOperator(client->Fd))
    {
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 482 "
            + client->Nickname + " " + channelName + " :You're not channel operator\r\n");
        return ;
    }

    // collect params after the modestring
    std::vector<std::string> args;
    std::string token;
    while (ss >> token)
        args.push_back(token);
    size_t argIdx = 0;

    char sign = '+';
    char lastSign = '\0';
    std::string appliedModes;
    std::string appliedParams;

    for (size_t i = 0; i < modeStr.size(); i++)
    {
        char c = modeStr[i];
        if (c == '+' || c == '-') { sign = c; continue ; }

        switch (c)
        {
            case 'i':
                channel.setInviteOnly(sign == '+');
                if (sign != lastSign) { appliedModes += sign; lastSign = sign; }
                appliedModes += 'i';
                break ;

            case 't':
                channel.setTopicRestricted(sign == '+');
                if (sign != lastSign) { appliedModes += sign; lastSign = sign; }
                appliedModes += 't';
                break ;

            case 'k':
                if (sign == '+')
                {
                    if (argIdx >= args.size())
                    {
                        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 461 "
                            + client->Nickname + " MODE :Not enough parameters\r\n");
                        break ;
                    }
                    channel.setKey(args[argIdx]);
                    appliedParams += " " + args[argIdx++];
                }
                else
                    channel.setKey("");
                if (sign != lastSign) { appliedModes += sign; lastSign = sign; }
                appliedModes += 'k';
                break ;

            case 'l':
                if (sign == '+')
                {
                    if (argIdx >= args.size())
                    {
                        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 461 "
                            + client->Nickname + " MODE :Not enough parameters\r\n");
                        break ;
                    }
                    int limit = std::atoi(args[argIdx].c_str());
                    if (limit > 0)
                    {
                        channel.setUserLimit((size_t)limit);
                        appliedParams += " " + args[argIdx];
                    }
                    argIdx++;
                }
                else
                    channel.setUserLimit(0);
                if (sign != lastSign) { appliedModes += sign; lastSign = sign; }
                appliedModes += 'l';
                break ;

            case 'o':
            {
                if (argIdx >= args.size())
                {
                    SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 461 "
                        + client->Nickname + " MODE :Not enough parameters\r\n");
                    break ;
                }
                Client *target = getClientByNickFromServer(args[argIdx]);
                if (!target || !channel.isClientInChannel(target->Fd))
                {
                    SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 441 "
                        + client->Nickname + " " + args[argIdx] + " " + channelName + " :They aren't on that channel\r\n");
                    argIdx++;
                    break ;
                }
                if (sign == '+')
                    channel.addOperator(target);
                else
                    channel.removeOperator(target->Fd);
                if (sign != lastSign) { appliedModes += sign; lastSign = sign; }
                appliedModes += 'o';
                appliedParams += " " + args[argIdx++];
                break ;
            }

            default:
                SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 472 "
                    + client->Nickname + " " + c + " :is unknown mode char to me\r\n");
                break ;
        }
    }

    if (!appliedModes.empty())
    {
        std::string broadcast = ":" + client->Nickname + "!" + client->Username + "@" + client->IpAddr
            + " MODE " + channelName + " " + appliedModes + appliedParams + "\r\n";
        channel.broadcastMessage(broadcast, -1);
    }
}

/**
 * parse param → get channelName + newTopic

if channelName empty → error 461
if channel doesn't exist → error 403
if client not in channel → error 442

if newTopic not provided:
    → just send back current topic (331 or 332)
    → done

if newTopic provided:
    if channel.isTopicRestricted() && !channel.isOperator(client->Fd):
        → error 482
    else:
        channel.setTopic(newTopic)
        broadcast to channel: ":nick!user@host TOPIC #channel :newtopic\r\n"
 */