#ifndef BOT_H
#define BOT_H

#include "Server.hpp"
#include <ctime>

class Bot
{
	private:
		int BotFd;
		std::string Buffer;
		std::string Nickname;
		std::string Password;
		time_t startTime;

	public:
		Bot(std::string password);
		void connect(int Port);
		int getFd() const;
		void SendRaw(const std::string &msg);
		void HandleEvent();
};

#endif