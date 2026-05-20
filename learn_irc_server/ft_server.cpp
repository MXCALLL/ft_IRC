#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
	//? create a socket
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	//? Defining Server Address
	sockaddr_in serverAddress; //* Declares a struct that will hold your server's address information
	serverAddress.sin_family = AF_INET; //* Tells the struct which addressing system to use. AF_INET = IPv4. This must match what you passed to socket() earlier.
	serverAddress.sin_port = htons(8080); //* Sets which port to listen on. htons() converts 8080 from your machine's byte order to network byte order
	serverAddress.sin_addr.s_addr = INADDR_ANY; //* Sets which IP address to listen on. INADDR_ANY means "accept connections arriving on any network interface" — wifi, ethernet, loopback, all of them.

	//? bind the socket to an IP / Port
	bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	//* serverSocket => The file descriptor of the socket you created in step 1. You're telling bind() "attach the address to THIS socket".
	//* (struct sockaddr*)&serverAddress => This is the address struct you just filled in — the business card with your IP and port.
	//? But notice the cast (struct sockaddr*). Why cast it?
	//* sockaddr_in is IPv4-specific. bind() is a generic function that works with any address type — IPv4, IPv6, Unix sockets. So it expects a generic sockaddr* pointer. The cast just says "treat this IPv4 address struct as a generic address". The data inside doesn't change.
	//* sizeof(serverAddress) => Since bind() accepts different address types of different sizes, it needs to know how many bytes to read. sizeof(serverAddress) tells it the exact size of your struct.


	//todo: mark the socket for listening in
	listen(serverSocket, 5);
	//* the fd of your socket. Same as always — tells the function which socket to work with.
	//* the backlog; This is the maximum number of clients that can be waiting in the queue to be accepted before the OS starts rejecting new connections. (Think of it like a waiting room)

	//todo: accept a call
	//todo: close the listening socket
	//todo: while receiving - display message, echo message
	//todo: close socket

	return 0;
}
