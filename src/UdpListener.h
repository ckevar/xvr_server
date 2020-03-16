#ifndef UDP_LISTENER_H
#define UDP_LISTENER_H

#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>

#define MAX_CLIENTS 10

class UdpListener
{

public:

	UdpListener(const char* ipAddress, int port) :
		m_ipAddress(ipAddress), m_port(port) { }

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
	virtual void onMessageReceived(int clientSocket, const char* msg, int length);

	// Send a message to a client
	void sendToClient(int clientSocket, const char* msg, int length);

	// Broadcast a message from a client
	void broadcastToClients(int sendingClient, const char* msg, int length);

	// Allocate client in the client set
	void allocateClient(struct sockaddr_in client);

	// Deallocate client from the the client set
	void deallocateClient(unsigned client); 

private:

	const char*		m_ipAddress;	// IP Address server will run on
	int				m_port;			// Port # for the web service
	int				m_socket;		// Internal FD for the listening socket
	struct pollfd	m_master[1];		// Master file descriptor set
	struct sockaddr_in	m_clients[MAX_CLIENTS];		// Master file descriptor set
	int				available;				// amount of available clients
	bool 			running;		// switch to run and stop the server's while
};

#endif
