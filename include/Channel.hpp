#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <map>
# include <vector>
# include "Client.hpp"

class Channel
{
	private:
		std::string					_name;
		std::string					_topic;
		std::string					_key;
		bool						_inviteOnly;
		bool						_topicRestricted;
		size_t						_userLimit;
		std::map<int, Client*>		_clients;
		std::map<int, Client*>		_operators;
		std::vector<int>			_inviteList;

	public:
		Channel(std::string name);
		~Channel();

		std::string	getName() const;
		bool		isInviteOnly() const;
		size_t		getClientCount() const;
		std::string	getKey() const;
		size_t		getChannelUserLimit() const;
		std::string	getClientList();
		bool		isInvited(int fd);
		bool		isEmpty() const;
		std::string	getTopic() const;
		void		setTopic(std::string const newtopic);
		void		setInviteOnly(bool val);
		void		setKey(std::string key);
		void		setUserLimit(size_t limit);
		bool		isTopicRestricted() const;
		void		setTopicRestricted(bool val);

		//* Client Management *//
		void addClient(Client* client);
		void removeClient(int fd);
		bool isClientInChannel(int fd);
		void addToInviteList(int fd);
		void removeFromInviteList(int fd);
		Client *getClientByNickFromChannel(std::string nickname);

		//* Operator Management *//
		void addOperator(Client* client);
		void removeOperator(int fd);
		bool isOperator(int fd);

		//* Core Action *//
		void broadcastMessage(std::string msg, int senderFd);

};

#endif
