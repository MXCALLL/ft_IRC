#include "Server.hpp"

bool Server::isPrintable( std::string params){

	for (size_t i = 0; i < params.size(); i++){
		if (std::isprint(params[i])){
			return (true);
		}
	}

	return (false);
}
