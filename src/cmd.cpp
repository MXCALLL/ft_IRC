#include "../include/Server.hpp"

void Server::CmdPass( std::string param, Client *client ){
    if (!client)
        return ;

    if (client->Registered){
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 462 " + client->Nickname + " :You may not reregister\r\n");
        return ;
    }

    if (param.empty()){
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 461 * PASS :Not enough parameters\r\n");
        return ;
    }

    if (param != Password){
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 464 * :Password incorrect\r\n");
        return ;
    }

    client->PassAccepted = true;
    std::cout << "[IRCSERV]: fd " << client->Fd << " Password Accepted !!" << std::endl;
}

void Server::CmdNick( std::string param, Client *client ){
    if (!client)
        return ;

    if (!client->PassAccepted){
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 451 * :You have not sent PASS\r\n");
        return ;
    }

    if (param.empty()){
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 431 * :No nickname given\r\n");
        return ;
    }

    if (!std::isalpha(param[0]) && param[0] != '_' && param[0] != '-'){
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 432 * " + param + " :Erroneous nickname\r\n");
        return ;
    }

    for (size_t i = 0; i < param.size(); i++){
        if (!std::isalnum(param[i]) && param[i] != '_' && param[i] != '-'){
            SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 432 * " + param + " :Erroneous nickname\r\n");
            return ;
        }
    }

    if (NicknameInUse(param)){
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 433 * " + param + " :Nickname is already in use\r\n");
        return ;
    }

    std::string oldNick = client->Nickname;
    client->Nickname = param;
    std::cout << "[IRCSERV]: fd " << client->Fd << " Nickname set to " << param << std::endl;

    if (!client->Registered && !client->Username.empty()){
        client->Registered = true;
        WelcomeClient(client->Fd);
        std::cout << "[IRCSERV]: fd " << client->Fd << " Registration Complete !!" << std::endl;
    }
}

void Server::CmdUser( std::string param, Client *client ){
    if (!client)
        return ;

    if (!client->PassAccepted){
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 451 * :You have not sent PASS\r\n");
        return ;
    }

    if (client->Registered){
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 462 " + client->Nickname + " :You may not reregister\r\n");
        return ;
    }

    if (param.empty()){
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 461 * USER :Not enough parameters\r\n");
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
        SendReply(client->Fd, ":" + std::string(SERVER_NAME) + " 461 * USER :Not enough parameters\r\n");
        return ;
    }

    client->Username = username;
    client->Realname = realname;
    std::cout << "[IRCSERV]: fd " << client->Fd << " Username set to " << username << std::endl;

    if (!client->Registered && !client->Nickname.empty() && !client->Username.empty()){
        client->Registered = true;
        WelcomeClient(client->Fd);
        std::cout << "[IRCSERV]: fd " << client->Fd << " Registration Complete !!" << std::endl;
    }
}

//* 
void Server::CmdJoin( std::string param, Client *client )
{
    // TODO: Implement JOIN logic here
    std::cout << "[IRCSERV]: JOIN command received from fd " << client->Fd << " with param: " << param << std::endl;
}

//* 
void Server::CmdKick( std::string param, Client *client )
{
    // TODO: Implement KICK logic here
    std::cout << "[IRCSERV]: KICK command received from fd " << client->Fd << " with param: " << param << std::endl;
}

//* 
void Server::CmdInvite( std::string param, Client *client )
{
    // TODO: Implement INVITE logic here
    std::cout << "[IRCSERV]: INVITE command received from fd " << client->Fd << " with param: " << param << std::endl;
}
