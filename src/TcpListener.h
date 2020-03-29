#ifndef TCP_LISTENER_H
#define TCP_LISTENER_H

#include <sys/socket.h>
#include <poll.h>

#define MAX_CLIENTS 	20 	// players allowed
#define BUF_SIZE 	1024 	// message length
class TcpListener
{

public:

	TcpListener(const char* ipAddress, int port) :
		m_ipAddress(ipAddress), m_port(port) { ticketID = 0;}

	// Initialize the listener
	int init();

	// Run the listener
	int run();

	// Stop Listener
	void stop();

protected:

	// Handler for client connections
	virtual void onClientConnected(int clientSocket);

	// Handler for client disconnections
	virtual void onClientDisconnected(int clientSocket);

	// Handler for when a message is received from the client
	virtual void onMessageReceived(int clientSocket, const unsigned char* msg, int length);

	// Send a message to a client
	void sendToClient(int clientSocket, const unsigned char* msg, int length);

	// Broadcast a message from a client
	void broadcastToClients(int sendingClient, const unsigned char* msg, int length);

	// Temporary allocation
	void tmpAllocClient(int client);

	// Allocate client in the client set
	void allocateClient(int client);

	// Deallocate client from the the client set
	void deallocateClient(unsigned i);

private:

	const char*	m_ipAddress;	// IP Address server will run on
	int		m_port;			// Port # for the web service
	int		m_socket;		// Internal FD for the listening socket
	struct pollfd	m_master[MAX_CLIENTS + 1];		// Master file descriptor set
	unsigned char 	allocStatus[MAX_CLIENTS + 1];		// Allocation status are they temporary or fixed
	int		available;		// amount of available clients
	int 		ticketID;
	bool 		running;		// switch to run and stop the server's while
};

#endif
