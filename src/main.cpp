
#include "include/Server.hpp"



int main(int ac, std::vector<std::string> av){

    if (ac != 2){
        std::cerr << "[IRCSERVER]: Argument Count Not Correct !!" << std::endl;
        return (EXIT_FAILURE);
    }


    int Port = std::atoi(av[1].c_str());
    std::string Password = av[2];

    try
    {
        Server Server(Port, Password);
        Server.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "[IRCSERVER]: " << e.what() << '\n';
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}
