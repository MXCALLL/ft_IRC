#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <map>
# include <vector>
# include "Client.hpp"

class Channel
{
	private:
		std::string                 _name;
		std::string                 _topic;
		std::string                 _key;             //? Password for the channel (mode k)
		bool                        _inviteOnly;      //? Invite-only mode (mode i)
		bool                        _topicRestricted; //? Restrict topic changes to ops (mode t)
		size_t                      _userLimit;       //? Max users allowed (mode l)

		// We use pointers (Client*) so we don't accidentally copy the user's data!
		// We want to reference the exact same Client object that the Server is managing.
		std::map<int, Client*>      _clients;         //? Map of regular users (Key: fd, Value: Client pointer)
		std::map<int, Client*>      _operators;       //? Map of channel operators (Key: fd, Value: Client pointer)
		std::vector<std::string>	_inviteList;      //? vector of invite list of clients

	public:
		//* Constructor & Destructor *//
		Channel(std::string name);
		~Channel();

		//*  Getters *//
		std::string getName() const;
		bool getInviteOnly() const;
		std::string getClientList();
		bool isInvited(std::string nickname);

		//* Client Management *//
		void addClient(Client* client);
		void removeClient(int fd);
		bool isClientInChannel(int fd);
		void addToInviteList(std::string nickname);

		//* Operator Management *//
		void addOperator(Client* client);
		void removeOperator(int fd);
		bool isOperator(int fd);

		//* Core Action *//
		void broadcastMessage(std::string msg, int senderFd);
};

#endif
