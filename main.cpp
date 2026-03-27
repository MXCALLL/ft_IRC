
#include "include/Server.hpp"



int main(int ac, char **av){

    if (ac != 3){
        std::cerr << "[ERROR]: ./ircserv <Port> <Password>" << std::endl;
        return (EXIT_FAILURE);
    }

    int Port = std::atoi(av[1]);
    std::string Password = av[2];

    if (Port <= MAX_SYS_PORT || Port > MAX_PORT){
        std::cerr << "[IRCSERV]: Invalid Port Number !!" << std::endl;
        return (EXIT_FAILURE);
    }

    if (Password.empty()){
        std::cerr << "[IRCSERV]: Password Cannot Be Empty !!" << std::endl;
        return (EXIT_FAILURE);
    }

    try
    {
        Server Server(Port, Password);
        Server.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "[IRCSERV]: " << e.what() << '\n';
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}
