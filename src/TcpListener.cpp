#include "TcpListener.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <cstdio>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int TcpListener::init() {
	// Create a socket
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket < 0) {
		fprintf(stderr,"[ERROR:] cannot open socket");
	}

	// Bind the ip address and port to a socket
	struct sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(m_port);
	inet_pton(AF_INET, m_ipAddress, &hint.sin_addr);

	if (bind(m_socket, (sockaddr*)&hint, sizeof(hint)) < 0) {
		fprintf(stderr, "[ERROR:] at binding");
		return -1;
	}
	// Tell the socket is for listening
	if (listen(m_socket, SOMAXCONN) < 0) {
		fprintf(stderr, "[ERROR:] at listen");
		return -1;
	}

	// Create the master file descriptor set and assing -1 file descriptor, and no event
	for (int i = 0; i < (MAX_CLIENTS + 1); ++i) {
		m_master[i].fd = -1;
		m_master[i].events = 0;
	}

	// Add our first socket that we're interested in interacting with; the listening socket!
	// It's important that this socket is added for our server or else we won't 'hear' incoming
	// connections
	available = MAX_CLIENTS;
	m_master[0].fd = m_socket; // the index zero is our listening socket

	running = true;	// Enable server to run
	return 0;
}

int TcpListener::run() {
	// this will be changed by the \quit command (see below, bonus not in video!)
	unsigned clientID, i;
	int client, socketCount, tmpSock, bytesIn;
	unsigned char buf[BUF_SIZE];

	while (running) {
		//std::cout << "[DEBUG:] Available seats " << available << " out of " << MAX_CLIENTS << std::endl;
		m_master[0].events = (available > 0) ? POLLIN : 0;	// Updating the event based on the availability
		// See who's talking to us
		socketCount = poll(m_master, MAX_CLIENTS, -1);	// Wait for connections
		// Is it an inbound communication?
		if (m_master[0].revents == POLLIN) {
			// Accept a new connection
			client = accept(m_socket, nullptr, nullptr);
			// Add the new connection to the list of connected clients
			tmpAllocClient(client); // tcp allocation
			socketCount--;
			std::cerr << "{Allocation granted:} " << available << " free seats" << std::endl;
		}

		i = 1;
		// It's an inbound message
		// Loop through all the current connections / potential connect
		while (socketCount > 0) {

			if(m_master[i].revents == POLLIN) {
				memset(buf, 0, BUF_SIZE);
				tmpSock = m_master[i].fd;

				// Receive message
				bytesIn = recv(tmpSock, buf, BUF_SIZE, 0);
				if (bytesIn <= 0) {
					// Drop the client
					close(tmpSock);
					std::cerr << "{debug:} socket closed " << std::endl;
					deallocateClient(&tmpSock);
					std::cerr << "{debug:} deallocated " << std::endl;
					onClientDisconnected(tmpSock);
					std::cerr << "{debug:} memory updated correctly " << std::endl;
					std::cerr << "{Deallocation granted:} " << available << " free seats" << std::endl;
				}
				else {
					onMessageReceived(tmpSock, buf, bytesIn);
				}
				socketCount--;
			}
			i++;
		}
	}

	// Remove the listening socket from the master file descriptor set and close it
	// to prevent anyone else trying to connect.
	close(m_master[0].fd);

	i = 1;
	while (available != MAX_CLIENTS) {

		if (m_master[i].fd > -1) {
			close(m_master[i].fd);
			m_master[i].fd = -1;
			available++;
		}
		i++;
	}

	return 0;
}

void TcpListener::stop(){
	running = false;
}

void TcpListener::tmpAllocClient(int client) {
	unsigned i = 1;

	if (available == 1) {
		for (i = 1; i < MAX_CLIENTS; i++) {
			if (allocStatus[i] == 0) { // temporary allocated
				close(m_master[i].fd);
				m_master[i].fd = -1;
				m_master[i].events = 0;
			}
		}
		i = 1;
	}

	while(m_master[i].fd > -1) i++;
	available--;

	m_master[i].fd = client;
	m_master[i].events = POLLIN;
	allocStatus[i] = 0; // temporary allocated
}

void TcpListener::allocateClient(int client) {

	unsigned i = 1;

	while(m_master[i].fd != client) i++;
	allocStatus[i - 1] = 1; // fully allocated
}

void TcpListener::deallocateClient(int *client) {
	unsigned i = 1; // starts at 1, because internal listener socket is at 0
	struct pollfd tmp[MAX_CLIENTS];
	unsigned char tmpState[MAX_CLIENTS];
	while((m_master[i].fd != *client) && (i <= MAX_CLIENTS)) i++;
	if (i < MAX_CLIENTS) {
		available++;
		memcpy(tmp, m_master + i + 1, sizeof(struct pollfd) * (MAX_CLIENTS - available - i));
		memcpy(m_master + i, tmp, sizeof(struct pollfd) * (MAX_CLIENTS - available - i));

		m_master[available].fd = -1;
		m_master[available].events = 0;

		memcpy(tmpState, allocStatus + i, MAX_CLIENTS - available - i);
		memcpy(allocStatus + i - 1, tmpState, MAX_CLIENTS - available - i);

		allocStatus[i - 1] = 0;
		*client = i - 1;
	}
}


void TcpListener::sendToClient(int clientSocket, const unsigned char* msg, int length) {
	send(clientSocket, msg, length, 0);
}

void TcpListener::broadcastToClients(int sendingClient, const unsigned char* msg, int length) {
	unsigned i = 0;
	unsigned j = 0;
	int outSock;
	while((j < MAX_CLIENTS) && (i < available)){
		outSock = m_master[j + 1].fd;
		if ((allocStatus[j]) && (sendingClient != outSock)) {
			i++;
			sendToClient(outSock, msg, length);
		}
		j++;
	}
}

void TcpListener::onClientConnected(int clientSocket)
{

}

void TcpListener::onClientDisconnected(int clientSocket)
{

}

void TcpListener::onMessageReceived(int clientSocket, const unsigned char* msg, int length)
{

}
