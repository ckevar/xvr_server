#include "UdpListener.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <cstdio>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int UdpListener::init()
{
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
	fprintf(stderr, "%u\n", AF_INET);
	// Create the master file descriptor set and assing -1 file descriptor, and no event
	for (int i = 0; i < (MAX_CLIENTS + 1); ++i) {
		m_clients[i].sin_family = 0;
	}

	// Add our first socket that we're interested in interacting with; the listening socket!
	// It's important that this socket is added for our server or else we won't 'hear' incoming
	// connections
	available = MAX_CLIENTS;
	m_master[0].fd = m_socket; // the index zero is our listening socket

	running = true;	// Enable server to run
	return 0;
}

int UdpListener::run() {
	// this will be changed by the \quit command (see below, bonus not in video!)

	while (running) {
		std::cout << "[DEBUG:] Available seats " << available << " out of " << MAX_CLIENTS << std::endl;
		// m_master[0].events = (available > 0) ? POLLIN : 0;	// Updating the event based on the availability
		m_master[0].events = POLLIN;
		// See who's talking to us
		int socketCount = poll(m_master, 1, -1);	// Wait for connections
		// Is it an inbound communication?
		if (m_master[0].revents == POLLIN) {
			// Accept a new connection
			int client = 0;
			//int client = accept(m_socket, nullptr, nullptr);
			int n;
			unsigned len;
			unsigned char buffer[1024];
			struct sockaddr_in cliaddr;
			n = recvfrom(m_socket, (unsigned char *)buffer, 1024, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
			/*printf("Client: \n\tfamiliy: %d\n\tport: %d\n\t addr: %u\n\t zero: %s\n", 
			       cliaddr.sin_family, cliaddr.sin_port, cliaddr.sin_addr.s_addr, cliaddr.sin_zero);
			*/
			if (n == 28) {
				char cmd[8];
				//strcpy(cmd, buffer+20);
				if(strcmp((char *)(buffer + 20), "ALLok_ME") == 0) {
					fprintf(stderr, "Allocation requested\n");
					// allocateClient(cliaddr);
					// onClientConnected(&cliaddr);
					// socketCount--;
				} else if (strcmp((char *)(buffer + 20), "DeAok_ME") == 0) {
					fprintf(stderr, "Deallocation requested\n");
					// deallocateClient(cliaddr.sin_addr.s_addr);
				}
			}
			printf("n = %d, len = %d, msg: ", n, len);
			for (int i = 0; i < n; i++){
				printf("%d ", buffer[i]);
			}
			printf("\n");

			// Add the new connection to the list of connected clients
			//allocateClient(client);

		}

		int i = 1;
		// It's an inbound message
		// Loop through all the current connections / potential connect
		/*while (socketCount > 0) {

			if(m_master[i].revents == POLLIN) {
				char buf[4096];
				memset(buf, 0, 4096);
				int sock = m_master[i].fd;

				// Receive message
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0) {
					// Drop the client
					onClientDisconnected(sock);
					close(sock);
					deallocateClient(sock);
				}
				else {
					onMessageReceived(sock, buf, bytesIn);
				}
				socketCount--;
			}
			i++;
		}*/
	}

	// Remove the listening socket from the master file descriptor set and close it
	// to prevent anyone else trying to connect.
	close(m_master[0].fd);

	int i = 1;
	while (available != MAX_CLIENTS) {

		if (m_master[i].fd) {
			// Get the socket number
			close(m_master[i].fd);
			// Remove it from the master file list and close the socket
			m_master[i].fd = -1;
			available++;
		}
		i++;
	}

	return 0;
}

void UdpListener::stop(){
	running = false;
}

void UdpListener::allocateClient(struct sockaddr_in client) {
	unsigned i = 1;

	while(m_clients[i].sin_family > 0) i++;
	available--;
	m_clients[i] = client;
}

void UdpListener::deallocateClient(unsigned client) {
	unsigned i = 1; // starts at 1, because internal listener socket is at 0
	
	while(m_master[i].sin_addr.s_addr != client) i++;
	available++;
	
	m_master[i].fd = -1;
	m_master[i].events = 0;
}


void UdpListener::sendToClient(int clientSocket, const char* msg, int length) {
	send(clientSocket, msg, length, 0);
}

void UdpListener::broadcastToClients(int sendingClient, const char* msg, int length)
{
	for (int i = 1; i < available; i++) {
		int outSock = m_master[i].fd;
		if (outSock != sendingClient) {
			sendToClient(outSock, msg, length);
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
