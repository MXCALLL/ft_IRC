#include "../include/Server.hpp"

bool Server::isPrintable( std::string params){

        for (size_t i = 0; i < params.size(); i++){
                if (!std::isprint(static_cast<unsigned char>(params[i])) && params[i] != '\r' && params[i] != '\n'){
                        return (false);
                }
        }

        return (true);
}
