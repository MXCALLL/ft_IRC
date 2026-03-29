
#include "../include/Server.hpp"


bool Server::Signal = false;

Server::Server(){}

Server::~Server(){
    for (std::map<int, Client>::iterator it = Clients.begin(); it != Clients.end(); ++it){
        close(it->first);
    }
    if (listenSockFd >= 0)
        close(listenSockFd);
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


Server::Server(int _Port, std::string _Password) : Port(_Port), Password(_Password){

    SetupSocket( this->Port );

    struct pollfd pfd;
    pfd.fd = listenSockFd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    Fd.push_back(pfd);

    std::cout << "[IRCSERV]: Listen on Port " << this->Port << std::endl;
}


void Server::run( void ){

    signal(SIGINT, SignalHandler);
    signal(SIGQUIT, SignalHandler);

    while (!Signal){

        for (size_t i = 1; i < Fd.size(); i++){
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

        for (size_t i = 0; i < Fd.size(); i++){
            if (Fd[i].revents & POLLIN){
                if (Fd[i].fd == listenSockFd)
                    AcceptClient();
                else
                    ReceiveData(Fd[i].fd);
            }
            if (Fd[i].revents & POLLOUT){
                SendData(Fd[i].fd);
            }
        }

        PerformTimeouts();
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

void Server::ReceiveData( int fd ){

    char buffer[BUFFER_SIZE];
    std::memset(buffer, 0, BUFFER_SIZE);

    int bytes = recv(fd, buffer, BUFFER_SIZE - 1, 0);

    if (bytes <= 0){
        DisconnectClient(fd);
        return ;
    }

    Client *client = getClientByFd(fd);
    if (!client)
        return ;

    client->LastActivityTime = time(NULL);
    client->Buffer += std::string(buffer, bytes);

    size_t pos;
    while ((pos = client->Buffer.find("\n")) != std::string::npos){

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

void Server::PerformTimeouts( void ){
    time_t now = time(NULL);
    const int PING_TIMEOUT = 120;
    const int DEAD_TIMEOUT = 180;

    for (std::map<int, Client>::iterator it = Clients.begin(); it != Clients.end(); ){
        time_t lastActivity = it->second.LastActivityTime;
        time_t lastPing = it->second.LastPingTime;
        int fd = it->first;

        if (now - lastActivity > DEAD_TIMEOUT){
            std::cout << "[IRCSERV]: Client " << fd << " Ping Timeout !!" << std::endl;

            ++it;
            DisconnectClient(fd);
            continue;
        }
        else if (now - lastActivity > PING_TIMEOUT && now - lastPing > PING_TIMEOUT){
            SendReply(fd, "PING :" + std::string(SERVER_NAME) + "\r\n");

            it->second.LastPingTime = now;
        }
        ++it;
    }
}


void Server::HandleCommand( std::string cmd, int fd ){

    while (!cmd.empty() && (cmd[cmd.size() - 1] == '\r' || cmd[cmd.size() - 1] == '\n'))
        cmd.erase(cmd.size() - 1);

    if (cmd.empty())
        return;

    std::stringstream ss(cmd);
    std::string prefix, command, param;

    if (cmd[0] == ':') {
        ss >> prefix;
    }

    ss >> command;
    std::getline(ss, param);

    size_t first_non_space = param.find_first_not_of(' ');
    if (first_non_space != std::string::npos) {
        param = param.substr(first_non_space);
    } else {
        param = "";
    }

    for (size_t i = 0; i < command.size(); i++)
        command[i] = std::toupper(command[i]);

    if (!isPrintable(param)){
        SendReply(fd, ":" + std::string(SERVER_NAME) + " 400 * :Non-printable characters not allowed\r\n");
        return ;
    }

    if (command == "PASS")
        CmdPass(param, fd);
    else if (command == "NICK")
        CmdNick(param, fd);
    else if (command == "USER")
        CmdUser(param, fd);
    else if (command == "PING")
        CmdPing(param, fd);
    else if (command == "PONG")
        CmdPong(param, fd);
    else{
        Client *client = getClientByFd(fd);
        if (client && !client->Registered)
            SendReply(fd, ":" + std::string(SERVER_NAME) + " 451 * :You have not registered\r\n");
    }
}

void Server::CmdPass( std::string param, int fd ){

    Client *client = getClientByFd(fd);
    if (!client)
        return ;

    if (client->Registered){
        SendReply(fd, ":" + std::string(SERVER_NAME) + " 462 " + client->Nickname + " :You may not reregister\r\n");
        return ;
    }

    if (param.empty()){
        SendReply(fd, ":" + std::string(SERVER_NAME) + " 461 * PASS :Not enough parameters\r\n");
        return ;
    }

    if (param != Password){
        SendReply(fd, ":" + std::string(SERVER_NAME) + " 464 * :Password incorrect\r\n");
        return ;
    }

    client->PassAccepted = true;
    std::cout << "[IRCSERV]: fd " << fd << " Password Accepted !!" << std::endl;
}


void Server::CmdNick( std::string param, int fd ){

    Client *client = getClientByFd(fd);
    if (!client)
        return ;

    if (!client->PassAccepted){
        SendReply(fd, ":" + std::string(SERVER_NAME) + " 451 * :You have not sent PASS\r\n");
        return ;
    }

    if (param.empty()){
        SendReply(fd, ":" + std::string(SERVER_NAME) + " 431 * :No nickname given\r\n");
        return ;
    }

    if (!std::isalpha(param[0]) && param[0] != '_' && param[0] != '-'){
        SendReply(fd, ":" + std::string(SERVER_NAME) + " 432 * " + param + " :Erroneous nickname\r\n");
        return ;
    }

    for (size_t i = 0; i < param.size(); i++){
        if (!std::isalnum(param[i]) && param[i] != '_' && param[i] != '-'){
            SendReply(fd, ":" + std::string(SERVER_NAME) + " 432 * " + param + " :Erroneous nickname\r\n");
            return ;
        }
    }

    if (NicknameInUse(param)){
        SendReply(fd, ":" + std::string(SERVER_NAME) + " 433 * " + param + " :Nickname is already in use\r\n");
        return ;
    }

    std::string oldNick = client->Nickname;
    client->Nickname = param;
    std::cout << "[IRCSERV]: fd " << fd << " Nickname set to " << param << std::endl;

    if (!client->Registered && !client->Username.empty()){
        client->Registered = true;
        WelcomeClient(fd);
        std::cout << "[IRCSERV]: fd " << fd << " Registration Complete !!" << std::endl;
    }
}


void Server::CmdUser( std::string param, int fd ){

    Client *client = getClientByFd(fd);
    if (!client)
        return ;

    if (!client->PassAccepted){
        SendReply(fd, ":" + std::string(SERVER_NAME) + " 451 * :You have not sent PASS\r\n");
        return ;
    }

    if (client->Registered){
        SendReply(fd, ":" + std::string(SERVER_NAME) + " 462 " + client->Nickname + " :You may not reregister\r\n");
        return ;
    }

    if (param.empty()){
        SendReply(fd, ":" + std::string(SERVER_NAME) + " 461 * USER :Not enough parameters\r\n");
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

    if (username.empty()){
        SendReply(fd, ":" + std::string(SERVER_NAME) + " 461 * USER :Not enough parameters\r\n");
        return ;
    }

    client->Username = username;
    client->Realname = realname;
    std::cout << "[IRCSERV]: fd " << fd << " Username set to " << username << std::endl;

    if (!client->Registered && !client->Nickname.empty() && !client->Username.empty()){
        client->Registered = true;
        WelcomeClient(fd);
        std::cout << "[IRCSERV]: fd " << fd << " Registration Complete !!" << std::endl;
    }
}


void Server::CmdPing( std::string param, int fd ){
    Client *client = getClientByFd(fd);
    if (!client)
        return ;

    if (param.empty()){
        SendReply(fd, ":" + std::string(SERVER_NAME) + " 409 * :No origin specified\r\n");
        return ;
    }

    SendReply(fd, "PONG " + std::string(SERVER_NAME) + " :" + param + "\r\n");
}

void Server::CmdPong( std::string param, int fd ){
    Client *client = getClientByFd(fd);
    if (!client)
        return ;

    (void)param;
    client->LastActivityTime = time(NULL);
}


Client *Server::getClientByFd( int fd ){
    if (Clients.count(fd))
        return &Clients[fd];
    return NULL;
}

void Server::SendReply( int fd, std::string msg ){
    Client *client = getClientByFd(fd);
    if (client){
        client->OutBuffer += msg;
    }
}

bool Server::NicknameInUse( std::string nickname ){
    for (std::map<int, Client>::iterator it = Clients.begin(); it != Clients.end(); ++it){
        if (it->second.Nickname == nickname)
            return true;
    }
    return false;
}

void Server::WelcomeClient( int fd ){

    Client *client = getClientByFd(fd);
    if (!client)
        return ;

    std::string nick = client->Nickname;
    std::string prefix = ":" + std::string(SERVER_NAME) + " ";

    SendReply(fd, prefix + "001 " + nick + " :Welcome to the IRC Network, " +
        nick + "!" + client->Username + "@" + client->IpAddr + "\r\n");
    SendReply(fd, prefix + "002 " + nick + " :Your host is " + SERVER_NAME + ", running version 1.0\r\n");
    SendReply(fd, prefix + "003 " + nick + " :This server was created today\r\n");
    SendReply(fd, prefix + "004 " + nick + " " + SERVER_NAME + " 1.0 o o\r\n");
}
