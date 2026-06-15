#include "../include/Channel.hpp"

Channel::Channel(std::string name) : _name(name), _topic(""), _key(""), _inviteOnly(false), _topicRestricted(false), _userLimit(0)
{

}

Channel::~Channel() {}

std::string Channel::getName() const
{
	return _name;
}

bool Channel::isInviteOnly() const
{
	return _inviteOnly;
}

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

size_t Channel::getClientCount() const
{
	return _clients.size();
}

std::string Channel::getKey() const
{
	return _key;
}

size_t Channel::getChannelUserLimit() const
{
	return _userLimit;
}

std::string Channel::getTopic() const
{
	return _topic;
}

void Channel::setTopic(std::string const newtopic)
{
	_topic = newtopic;
}

bool Channel::isTopicRestricted() const
{
	return _topicRestricted;
}

void Channel::setTopicRestricted(bool val)
{
	_topicRestricted = val;
}

void Channel::setInviteOnly(bool val)
{
	_inviteOnly = val;
}

void Channel::setKey(std::string key)
{
	_key = key;
}

void Channel::setUserLimit(size_t limit)
{
	_userLimit = limit;
}

void Channel::addClient(Client* client)
{
	if (client && _clients.count(client->Fd) == 0)
		_clients[client->Fd] = client;
}

void Channel::removeClient(int fd)
{
	_clients.erase(fd);
	removeOperator(fd);
}

bool Channel::isClientInChannel(int fd)
{
	return _clients.count(fd) > 0;
}

void Channel::addToInviteList(int fd)
{
	_inviteList.push_back(fd);
}

void Channel::removeFromInviteList(int fd)
{
	for (size_t i = 0; i < _inviteList.size(); i++)
	{
		if (fd == _inviteList.at(i))
		{
			_inviteList.erase(_inviteList.begin() + i);
			break ;
		}
	}
}

Client* Channel::getClientByNickFromChannel(std::string nickname)
{
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second->Nickname == nickname)
            return it->second;
    }
    return NULL;
}

bool Channel::isInvited(int fd)
{
	for (size_t i = 0; i < _inviteList.size(); ++i)
	{
		if (fd == _inviteList.at(i))
			return true;
	}
	return false;
}

bool Channel::isEmpty() const
{
	return _clients.empty();
}

void Channel::addOperator(Client* client)
{
	if (client)
		_operators[client->Fd] = client;
}

void Channel::removeOperator(int fd)
{
	_operators.erase(fd);
}

bool Channel::isOperator(int fd)
{
	return _operators.count(fd) > 0;
}

void Channel::broadcastMessage(std::string msg, int senderFd)
{
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->first != senderFd)
			it->second->OutBuffer += msg;
	}
}
