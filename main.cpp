#include "include/Server.hpp"

int main(int argc, char const *argv[])
{
    if (argc != 3)
	{
        std::cerr << "[ERROR]: ./ircserv <Port> <Password>" << std::endl;
        return EXIT_FAILURE;
    }

    std::istringstream ss(argv[1]);
    int Port;
    ss >> Port;
    std::string Password = argv[2];

    if (Port <= MAX_SYS_PORT || Port > MAX_PORT)
	{
        std::cerr << "[IRCSERV]: Invalid Port Number !!" << std::endl;
        return (EXIT_FAILURE);
    }

    if (Password.empty())
	{
        std::cerr << "[IRCSERV]: Password Cannot Be Empty !!" << std::endl;
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < Password.size(); ++i){
        if (std::isspace(Password[i])){
            std::cerr << "[IRCSERV]: The Password should be without spaces !!" << std::endl;
            return EXIT_FAILURE;
        }
    }

    try
    {
        Server Server(Port, Password);
        Bot IRCBot(Password);
        IRCBot.connect(Port);
        Server.run( IRCBot );
    }
    catch(const std::exception& e)
    {
        std::cerr << "[IRCSERV]: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
