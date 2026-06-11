#include "../include/Server.hpp"

Bot::Bot(std::string password) : Password(password), startTime(time(NULL)) {}

void Bot::SendRaw(const std::string &msg) {

  send(this->BotFd, msg.c_str(), msg.size(), 0);
}

Bot::~Bot(){}

void Bot::HandleEvent()
{
	char buffer[1024];
	std::memset(buffer, 0, sizeof(buffer));

	int bytes = recv(BotFd, buffer, sizeof(buffer) - 1, 0);
	if (bytes <= 0)
		return ;

	Buffer += std::string(buffer, bytes);

	size_t pos;
	while ((pos = Buffer.find('\n')) != std::string::npos)
	{
		std::string line = Buffer.substr(0, pos);
		Buffer.erase(0, pos + 1);
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.empty())
			continue ;

		std::istringstream ss(line);
		std::string prefix, command, param;

		if (line[0] == ':')
			ss >> prefix;
		ss >> command;
		std::getline(ss, param);

		if (command == "PING")
			SendRaw("PONG" + param + "\r\n");
		else if (command == "PRIVMSG")
		{
			size_t excl = prefix.find('!');
			if (excl == std::string::npos)
				continue ;
			std::string nick = prefix.substr(1, excl - 1);

			std::istringstream pss(param);
			std::string target;
			pss >> target;

			size_t colon = param.find(':');
			if (colon == std::string::npos)
				continue ;
			std::string msg = param.substr(colon + 1);

			if (target.empty() || target[0] == '#' || target[0] == '&')
				continue ;

			if (msg == "!ping")
				SendRaw("PRIVMSG " + nick + " :PONG! I am alive.\r\n");
			else if (msg == "!uptime")
			{
				time_t elapsed = time(NULL) - startTime;
				int hours   = elapsed / 3600;
				int minutes = (elapsed % 3600) / 60;
				int seconds = elapsed % 60;

				std::ostringstream oss;
				oss << hours << "h " << minutes << "m " << seconds << "s";
				SendRaw("PRIVMSG " + nick + " :Server uptime: " + oss.str() + "\r\n");
			}
			else if (msg == "!help")
			{
				SendRaw("PRIVMSG " + nick + " :Commands:\r\n");
				SendRaw("PRIVMSG " + nick + " :  !ping   - Check if bot is alive\r\n");
				SendRaw("PRIVMSG " + nick + " :  !uptime - Show server uptime\r\n");
				SendRaw("PRIVMSG " + nick + " :  !help   - Show this message\r\n");
			}
			else
				SendRaw("PRIVMSG " + nick + " :Unknown command. Type !help\r\n");
		}
	}
}

int Bot::getFd() const { return (this->BotFd); }

void Bot::connect(int Port) {

  BotFd = socket(AF_INET, SOCK_STREAM, 0);
  if (BotFd < 0)
    throw std::runtime_error("Error On Socket !!");

  sockaddr_in serv;
  std::memset(&serv, 0, sizeof(serv));

  serv.sin_family = AF_INET;
  serv.sin_port = htons(Port);
  serv.sin_addr.s_addr = inet_addr(ADDR);

  if (::connect(BotFd, reinterpret_cast<sockaddr *>(&serv), sizeof(serv)))
    throw std::runtime_error("Error on Connect !!");

  SendRaw("PASS " + Password + "\r\n");
  SendRaw("NICK BOT\r\n");
  SendRaw("USER bot 0 * :IRC bot\r\n");

  this->Nickname = "BOT";
  std::cout << "[BOT]: Initialization Sucsess" << std::endl;
}
