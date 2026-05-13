#include "include/Server.hpp"

int main(int argc, char const *argv[])
{
    if (argc != 3)
	{
        std::cerr << "[ERROR]: ./ircserv <Port> <Password>" << std::endl;
        return (EXIT_FAILURE);
    }

    int Port = std::atoi(argv[1]);
    std::string Password = argv[2];

    if (Port <= MAX_SYS_PORT || Port > MAX_PORT)
	{
        std::cerr << "[IRCSERV]: Invalid Port Number !!" << std::endl;
        return (EXIT_FAILURE);
    }

    if (Password.empty())
	{
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
