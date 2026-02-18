
#include "../include/Server.hpp"


bool Server::Signal = false;

Server::Server(){}

Server::~Server(){
    for (size_t i = 0; i < Clients.size(); i++){
        close(Clients[i].getFd());
    }
    if (listenSockFd >= 0)
        close(listenSockFd);
}

void Server::SignalHandler( int signum ){
    (void)signum;
    std::cout << "\n[IRCSERVER]: Signal Received, Shutting Down !!" << std::endl;
    Signal = true;
}

void Server::Socketreuse( int fd ){

    int op = 1;
    if (setsockopt(listenSockFd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op)) < 0){
        close (fd);
        throw std::runtime_error("Error On Sockopt ADDR !!");
    }

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

    Socketreuse(listenSockFd);
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

    std::cout << "[IRCSERVER]: Listen on Port " << this->Port << std::endl;
}


void Server::run( void ){

    signal(SIGINT, SignalHandler);
    signal(SIGQUIT, SignalHandler);

    while (!Signal){

        int ret = poll(&Fd[0], Fd.size(), -1);
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
        }
    }

    stop();
}

void Server::stop( void ){
    for (size_t i = 0; i < Clients.size(); i++){
        close(Clients[i].getFd());
    }
    Clients.clear();
    Fd.clear();
    if (listenSockFd >= 0){
        close(listenSockFd);
        listenSockFd = -1;
    }
    std::cout << "[IRCSERVER]: Server Stopped !!" << std::endl;
}

void Server::AcceptClient( void ){

    sockaddr_in clientAddr;
    socklen_t   clientLen = sizeof(clientAddr);

    int clientFd = accept(listenSockFd, reinterpret_cast<sockaddr *>(&clientAddr), &clientLen);
    if (clientFd < 0){
        std::cerr << "[IRCSERVER]: Error On Accept !!" << std::endl;
        return ;
    }

    SetNonBlocking(clientFd);

    struct pollfd pfd;
    pfd.fd = clientFd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    Fd.push_back(pfd);

    Client newClient(clientFd, inet_ntoa(clientAddr.sin_addr));
    Clients.push_back(newClient);

    std::cout << "[IRCSERVER]: New Connection from " << inet_ntoa(clientAddr.sin_addr)
              << " on fd " << clientFd << std::endl;
}

void Server::DisconnectClient( int fd ){

    std::cout << "[IRCSERVER]: Client Disconnected fd " << fd << std::endl;

    close(fd);

    for (size_t i = 0; i < Clients.size(); i++){
        if (Clients[i].getFd() == fd){
            Clients.erase(Clients.begin() + i);
            break ;
        }
    }

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

    client->appendBuffer(std::string(buffer, bytes));

    std::string buf = client->getBuffer();
    size_t pos;

    while ((pos = buf.find("\n")) != std::string::npos){

        std::string line = buf.substr(0, pos);
        buf = buf.substr(pos + 1);
        client->clearBuffer();
        client->appendBuffer(buf);

        if (!line.empty() && line[line.size() - 1] == '\r')
            line = line.substr(0, line.size() - 1);

        if (!line.empty())
            HandleCommand(line, fd);

        buf = client->getBuffer();
    }
}

void Server::HandleCommand( std::string cmd, int fd ){

    std::string command;
    std::string param;

    size_t space = cmd.find(' ');
    if (space != std::string::npos){
        command = cmd.substr(0, space);
        param = cmd.substr(space + 1);
    }
    else
        command = cmd;

    for (size_t i = 0; i < command.size(); i++)
        command[i] = std::toupper(command[i]);

    if (command == "CAP")
        return ;

    if (command == "PASS")
        CmdPass(param, fd);
    else if (command == "NICK")
        CmdNick(param, fd);
    else if (command == "USER")
        CmdUser(param, fd);
    else{
        Client *client = getClientByFd(fd);
        if (client && !client->getRegistered())
            SendReply(fd, ":" + std::string(SERVER_NAME) + " 451 * :You have not registered\r\n");
    }
}

void Server::CmdPass( std::string param, int fd ){

    Client *client = getClientByFd(fd);
    if (!client)
        return ;

    if (client->getRegistered()){
        SendReply(fd, ":" + std::string(SERVER_NAME) + " 462 " + client->getNickname() + " :You may not reregister\r\n");
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

    client->setPassAccepted(true);
    std::cout << "[IRCSERVER]: fd " << fd << " Password Accepted !!" << std::endl;
}


void Server::CmdNick( std::string param, int fd ){

    Client *client = getClientByFd(fd);
    if (!client)
        return ;

    if (!client->getPassAccepted()){
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

    std::string oldNick = client->getNickname();
    client->setNickname(param);
    std::cout << "[IRCSERVER]: fd " << fd << " Nickname set to " << param << std::endl;

    if (!client->getRegistered() && !client->getUsername().empty()){
        client->setRegistered(true);
        WelcomeClient(fd);
        std::cout << "[IRCSERVER]: fd " << fd << " Registration Complete !!" << std::endl;
    }
}


void Server::CmdUser( std::string param, int fd ){

    Client *client = getClientByFd(fd);
    if (!client)
        return ;

    if (!client->getPassAccepted()){
        SendReply(fd, ":" + std::string(SERVER_NAME) + " 451 * :You have not sent PASS\r\n");
        return ;
    }

    if (client->getRegistered()){
        SendReply(fd, ":" + std::string(SERVER_NAME) + " 462 " + client->getNickname() + " :You may not reregister\r\n");
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

    client->setUsername(username);
    client->setRealname(realname);
    std::cout << "[IRCSERVER]: fd " << fd << " Username set to " << username << std::endl;

    if (!client->getRegistered() && !client->getNickname().empty()){
        client->setRegistered(true);
        WelcomeClient(fd);
        std::cout << "[IRCSERVER]: fd " << fd << " Registration Complete !!" << std::endl;
    }
}


Client *Server::getClientByFd( int fd ){

    for (size_t i = 0; i < Clients.size(); i++){
        if (Clients[i].getFd() == fd)
            return (&Clients[i]);
    }
    return (NULL);
}

void Server::SendReply( int fd, std::string msg ){
    send(fd, msg.c_str(), msg.size(), 0);
}

bool Server::NicknameInUse( std::string nickname ){

    for (size_t i = 0; i < Clients.size(); i++){
        if (Clients[i].getNickname() == nickname)
            return (true);
    }
    return (false);
}

void Server::WelcomeClient( int fd ){

    Client *client = getClientByFd(fd);
    if (!client)
        return ;

    std::string nick = client->getNickname();
    std::string prefix = ":" + std::string(SERVER_NAME) + " ";

    SendReply(fd, prefix + "001 " + nick + " :Welcome to the IRC Network, " +
        nick + "!" + client->getUsername() + "@" + client->getIpAddr() + "\r\n");
    SendReply(fd, prefix + "002 " + nick + " :Your host is " + SERVER_NAME + ", running version 1.0\r\n");
    SendReply(fd, prefix + "003 " + nick + " :This server was created today\r\n");
    SendReply(fd, prefix + "004 " + nick + " " + SERVER_NAME + " 1.0 o o\r\n");
}
