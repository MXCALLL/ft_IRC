#include "../include/Server.hpp"

bool Server::Signal = false;

//* default constructor:
Server::Server(){}

//* parameterized constructor:
Server::Server(int _Port, std::string _Password) : Port(_Port), Password(_Password)
{
	SetupSocket( this->Port );

	struct pollfd pfd;
	pfd.fd = listenSockFd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	Fd.push_back(pfd);

	std::cout << "[IRCSERV]: Listen on Port " << this->Port << std::endl;
}

//* destructor:
Server::~Server()
{
	for (std::map<int, Client>::iterator it = Clients.begin(); it != Clients.end(); ++it)
		close(it->first);

	if (listenSockFd >= 0)
		close(listenSockFd);
}

//* The Main Loop
void Server::run( void )
{

	signal(SIGINT, SignalHandler);
	signal(SIGQUIT, SignalHandler);

	while (!Signal)
	{
		for (size_t i = 1; i < Fd.size(); i++)
		{
			Client *client = getClientByFd(Fd[i].fd);
			if (client && !client->OutBuffer.empty())
				Fd[i].events |= POLLOUT;
			else
				Fd[i].events &= ~POLLOUT;
		}

		int ret = poll(&Fd[0], Fd.size(), 1000);

		if (ret < 0 && !Signal)
			throw std::runtime_error("Error On Poll !!");
		if (Signal)
			break ;

		for (size_t i = 0; i < Fd.size(); i++)
		{
			if (Fd[i].revents & POLLIN)
			{
				if (Fd[i].fd == listenSockFd)
					AcceptClient();
				else
					ReceiveData(Fd[i].fd);
			}
			if (Fd[i].revents & POLLOUT)
				SendData(Fd[i].fd);
		}
	}
	stop();
}

void Server::stop( void ){
	for (std::map<int, Client>::iterator it = Clients.begin(); it != Clients.end(); ++it){
		close(it->first);
	}
	Clients.clear();
	Fd.clear();
	if (listenSockFd >= 0){
		close(listenSockFd);
		listenSockFd = -1;
	}
	std::cout << "[IRCSERV]: Server Stopped !!" << std::endl;
}

void Server::SignalHandler( int signum ){
	(void)signum;
	std::cout << "\n[IRCSERV]: Shutting Down !!" << std::endl;
	Signal = true;
}

void Server::SetNonBlocking( int fd ){

	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0){
		close(fd);
		throw std::runtime_error("Error On Fcntl !!");
	}
}

void Server::SetupSocket( int Port ){

	listenSockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSockFd <  0)
	{
		close (listenSockFd);
		throw std::runtime_error("Error On Socket !!");
	}

	int op = 1;
	if (setsockopt(listenSockFd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op)) < 0){
		close(listenSockFd);
		throw std::runtime_error("Error On Sockopt ADDR !!");
	}

	SetNonBlocking(listenSockFd);

	sockaddr_in serv;
	std::memset(&serv, 0, sizeof(serv));

	serv.sin_family = AF_INET;
	serv.sin_port = htons(Port);
	serv.sin_addr.s_addr = inet_addr(ADDR);

	if (bind(listenSockFd, reinterpret_cast<sockaddr *>(&serv), sizeof(serv)) < 0){

		close(listenSockFd);
		throw std::runtime_error("Error On Bind !!");
	}

	if (listen(listenSockFd, MAX_PENDING_CONNECTIONS) < 0){

		close(listenSockFd);
		throw std::runtime_error("Error On Listen !!");
	}

}

void Server::AcceptClient( void ){

	sockaddr_in clientAddr;
	socklen_t   clientLen = sizeof(clientAddr);

	int clientFd = accept(listenSockFd, reinterpret_cast<sockaddr *>(&clientAddr), &clientLen);
	if (clientFd < 0){
		std::cerr << "[IRCSERV]: Error On Accept !!" << std::endl;
		return ;
	}

	SetNonBlocking(clientFd);

	struct pollfd pfd;
	pfd.fd = clientFd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	Fd.push_back(pfd);

	Client newClient(clientFd, inet_ntoa(clientAddr.sin_addr));
	Clients[clientFd] = newClient;

	std::cout << "[IRCSERV]: New Connection from " << inet_ntoa(clientAddr.sin_addr)
			  << " on fd " << clientFd << std::endl;
}

//* The Buffer Manager
void Server::ReceiveData( int fd )
{
	char buffer[BUFFER_SIZE];
	std::memset(buffer, 0, BUFFER_SIZE);

	int bytes = recv(fd, buffer, BUFFER_SIZE - 1, 0);

	if (bytes == 0)
	{
		DisconnectClient(fd);
		return ;
	}
	if (bytes < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return ;
		DisconnectClient(fd);
		return ;
	}

	Client *client = getClientByFd(fd);
	if (!client)
		return ;

	client->Buffer += std::string(buffer, bytes);

	size_t pos;
	while ((pos = client->Buffer.find("\n")) != std::string::npos)
	{
		std::string line = client->Buffer.substr(0, pos);
		client->Buffer.erase(0, pos + 1);

		if (!line.empty() && line[line.size() - 1] == '\r')
			line = line.substr(0, line.size() - 1);

		if (!line.empty())
			HandleCommand(line, fd);
	}
}

void Server::SendData( int fd ){
	Client *client = getClientByFd(fd);
	if (!client)
		return ;

	std::string out = client->OutBuffer;
	if (out.empty())
		return ;

	int bytes = send(fd, out.c_str(), out.size(), 0);
	if (bytes > 0){
		client->OutBuffer.erase(0, bytes);
	} else if (bytes < 0) {
		if (errno != EWOULDBLOCK && errno != EAGAIN) {
			std::cerr << "[IRCSERV]: Send error on fd " << fd << std::endl;
			DisconnectClient(fd);
		}
	}
}

void Server::DisconnectClient( int fd ){

	std::cout << "[IRCSERV]: Client Disconnected fd " << fd << std::endl;

	close(fd);

	Clients.erase(fd);

	for (size_t i = 0; i < Fd.size(); i++){
		if (Fd[i].fd == fd){
			Fd.erase(Fd.begin() + i);
			break ;
		}
	}
}

//* The Execution
void Server::HandleCommand( std::string cmd, int fd )
{

	while (!cmd.empty() && (cmd[cmd.size() - 1] == '\r' || cmd[cmd.size() - 1] == '\n'))
		cmd.erase(cmd.size() - 1);

	if (cmd.empty())
		return;

	std::stringstream ss(cmd);
	std::string prefix, command, param;

	if (cmd[0] == ':')
		ss >> prefix;

	ss >> command;
	std::getline(ss, param);

	size_t first_non_space = param.find_first_not_of(' ');
	if (first_non_space != std::string::npos)
		param = param.substr(first_non_space);
	else
		param = "";

	for (size_t i = 0; i < command.size(); i++)
		command[i] = std::toupper(command[i]);

	if (!isPrintable(param))
	{
		SendReply(fd, ":" + std::string(SERVER_NAME) + " 400 * :Non-printable characters not allowed\r\n");
		return ;
	}

	Client *client = getClientByFd(fd);
	if (!client)
		return;

	if (command == "PASS")
		CmdPass(param, client);
	else if (command == "NICK")
		CmdNick(param, client);
	else if (command == "USER")
		CmdUser(param, client);
	else if (!client->Registered)
		SendReply(fd, ":" + std::string(SERVER_NAME) + " 451 * :You have not registered\r\n");
	//! === this part below is for (join/kick/invite) commands !//
	else if (command == "JOIN")
		CmdJoin(param, client);  //todo
    else if (command == "KICK")
		CmdKick(param, client);  //todo
    else if (command == "INVITE")
		CmdInvite(param, client);//todo
}
