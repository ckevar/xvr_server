#include "UdpListener.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <cstdio>
#include <arpa/inet.h>
#include <unistd.h>

int UdpListener::init() {
	// Create a socket
	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket < 0) {
		fprintf(stderr,"[ERROR:] cannot open socket");
	}

	// Bind the ip address and port to a socket
	struct sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(m_port);
	hint.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_pton(AF_INET, m_ipAddress, &hint.sin_addr);

	if (bind(m_socket, (sockaddr*)&hint, sizeof(hint)) < 0) {
		fprintf(stderr, "[ERROR:] at binding\n");
		return -1;
	}
	// Create the master file descriptor set and assing -1 file descriptor, and no event
	for (int i = 0; i < MAX_CLIENTS; ++i)
		m_clients[i].addr.sin_family = 0;

	// Add our first socket that we're interested in interacting with; the listening socket!
	// It's important that this socket is added for our server or else we won't 'hear' incoming
	// connections
	available = MAX_CLIENTS;
	m_master[0].fd = m_socket; // the index zero is our listening socket

	running = true;	// Enable server to run
	return 0;
}

int UdpListener::run() {
	unsigned char buffer[64];
	int bytesRecv;
	struct sockaddr_udp client;
	char cmd[9];
	cmd[8] = 0;

	while (running) {
		std::cerr << "[DEBUG:] Available seats " << available << " out of " << MAX_CLIENTS << std::endl;
		m_master[0].events = (available > 0) ? POLLIN : 0;	// Updating the event based on the availability
		
		// See who's talking to us
		int socketCount = poll(m_master, 1, -1);	// Wait for connections
		
		// if it's an Incoming data?
		if (m_master[0].revents == POLLIN) {
			// Receive from a client
			bytesRecv = recvfrom(m_socket, buffer, 64, MSG_WAITALL, (struct sockaddr *) &client.addr, &client.len);
			/*printf("Client: \n\tfamiliy: %d\n\tport: %d\n\t addr: %u\n\t zero: %s\n", 
			       client.sin_family, client.sin_port, client.sin_addr.s_addr, client.sin_zero);
			*/
			if (bytesRecv == 28) {	// 28 is the size of the when client asks/for allocation
				memcpy(cmd, buffer + 20, 8);

				// did client ask for allocation?
				if(strcmp(cmd, "ALLok_ME") == 0) {
					fprintf(stderr, "Allocation requested\n");
					allocateClient(client);
					// onClientConnected(&client);
					bytesRecv = -1;
				// did client ask for deallocation?
				} else if (strcmp(cmd, "DeAok_ME") == 0) {
					fprintf(stderr, "Deallocation requested\n");
					deallocateClient(client.addr.sin_addr.s_addr);
					bytesRecv = -1;
				}
			}

			if (bytesRecv > -1) {

				printf("n = %d, len = %d, msg: ", bytesRecv, client.len);
				for (int i = 0; i < bytesRecv; i++){
					printf("%d ", buffer[i]);
				}
				printf("\n");
				// broadcastToClients(client.addr.sin_addr.s_addr, buffer, bytesRecv);
				broadcastToClients(10, buffer, bytesRecv);
			}

		}

	}

	// Remove the listening socket from the master file descriptor set and close it
	// to prevent anyone else trying to connect.
	close(m_master[0].fd);
	return 0;
}

void UdpListener::stop(){
	running = false;
}

void UdpListener::allocateClient(struct sockaddr_udp client) {
	unsigned i = 0;

	while(m_clients[i].addr.sin_family > 0) i++;
	// std::cerr << "i = " << std::endl;
	available--;
	m_clients[i] = client;
}

void UdpListener::deallocateClient(unsigned client) {
	unsigned i = 0; // starts at 1, because internal listener socket is at 0
	
	while(m_clients[i].addr.sin_addr.s_addr != client) i++;
	available++;
	
	m_clients[i].addr.sin_family = -1;
}


// void UdpListener::sendToClient(int clientSocket, const char* msg, int length) {
void UdpListener::sendToClient(const unsigned char *msg, int length, struct sockaddr_udp client) {
	sendto(m_socket, msg, length, 0, (struct sockaddr*) &client.addr, client.len);
	// send(clientSocket, msg, length, 0);
}

void UdpListener::broadcastToClients(unsigned sendingClient, const unsigned char* msg, int length)
{	
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (m_clients[i].addr.sin_family > 0) {
			if (m_clients[i].addr.sin_addr.s_addr != sendingClient) {
				sendToClient(msg, length, m_clients[i]);
			}
		}
	}
}

void UdpListener::onClientConnected(int clientSocket)
{

}

void UdpListener::onClientDisconnected(int clientSocket)
{

}

void UdpListener::onMessageReceived(int clientSocket, const char* msg, int length)
{

}
