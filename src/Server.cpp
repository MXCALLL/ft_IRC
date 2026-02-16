
#include "include/Server.hpp"


Server::Server(){}

Server::~Server(){}

void Server::Socketreuse( int fd ){

    int op = 1;
    if (setsockopt(listenSockFd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op)) < 0){
        close (fd);
        throw std::runtime_error("Error On Sockopt ADDR !!");
    }

    if (setsockopt(listenSockFd, SOL_SOCKET, SO_REUSEPORT, &op, sizeof(op)) < 0){
        close (fd);
        throw std::runtime_error("Error On Sockopt PORT !!");
    }

}

void Server::SetNonBlocking( int fd ){

    if (fcntl(fd, O_NONBLOCK, 0) < 0){
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


Server::Server(int Port, std::string Password){

    SetupSocket( Port );


    std::cout << "[IRCSERVER]: Listen on Port " << Port << std::endl;
}


void Server::run( void ){

}
