#include "../include/Channel.hpp"

//? Constructor
Channel::Channel(std::string name) : _name(name), _topic(""), _key(""), _inviteOnly(false), _topicRestricted(false), _userLimit(0)
{

}

//? Destructor
Channel::~Channel() {}

//! --- Getters ---

//? Gets the channel's name
std::string Channel::getName() const
{
	return _name;
}

//? check if this channel is invite only
bool Channel::getInviteOnly() const
{
	return _inviteOnly;
}

//? Generates a space-separated list of all users in the channel, with operators prefixed by '@'
std::string Channel::getClientList()
{
	std::string list = "";

	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (isOperator(it->first))
			list += "@";
		list += it->second->Nickname + " ";
	}

	if (!list.empty() && list[list.size() - 1] == ' ')
		list = list.substr(0, list.size() - 1);

	return list;
}

//! --- Client Management ---

//? Adds a new client to the channel's client map
void Channel::addClient(Client* client)
{
	if (client && _clients.count(client->Fd) == 0)
		_clients[client->Fd] = client;
}

//? Removes a client from the channel and strips their operator status if they had it
void Channel::removeClient(int fd)
{
	_clients.erase(fd);
	removeOperator(fd);
}

//? Checks if a specific client is currently in the channel
bool Channel::isClientInChannel(int fd)
{
	return _clients.count(fd) > 0;
}

//? add a client to invite list
void Channel::addToInviteList(std::string nickname)
{
	_inviteList.push_back(nickname);
}

//? get a client from current channel by nickname
Client* Channel::getClientByNick(std::string nickname)
{
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second->Nickname == nickname)
            return it->second;
    }
    return NULL;
}

//? Checks if a specific client is currently in the invited list
bool Channel::isInvited(std::string nickname)
{
	for (size_t i = 0; i < _inviteList.size(); ++i)
	{
		if (nickname == _inviteList.at(i))
			return true;
	}
	return false;
}

//! --- Operator Management ---

//? Checks if a specific operator is currently in the channel
void Channel::addOperator(Client* client)
{
	if (client)
		_operators[client->Fd] = client;
}

//? Removes a client's operator privileges for this channel
void Channel::removeOperator(int fd)
{
	_operators.erase(fd);
}

//? Checks if a specific client has operator privileges in this channel
bool Channel::isOperator(int fd)
{
	return _operators.count(fd) > 0;
}

//! --- Core Action ---

//? Forwards a message to every client in the channel except the sender
void Channel::broadcastMessage(std::string msg, int senderFd) //* -1 = send to ALL including sender
{
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->first != senderFd)
			it->second->OutBuffer += msg;
	}
}
