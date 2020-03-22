#ifndef CORONA_SHOOT_H
#define CORONA_SHOOT_H

#include "TcpListener.h"

#define PDU_FULL_LEN 	11 // XVR total pdu length
#define PDU_VECTOR_LEN 	20 // XVR total vector length
#define PDU_BASE_LEN	21 // XVR position where it starts
#define PDU_TYPE	16 // XVR type 5:char, 8 float32_t
#define PDU_REAL_T	8  // XVR real float32_t type
#define PDU_CHAR_T	5  // XVR char type


// Global Variables
extern float listReqPDU, idPDU;

class CoronaShoot : public TcpListener
{
public:

	CoronaShoot(const char* ipAddress, int port) :
		TcpListener(ipAddress, port) {
			totalPlayers = 0;
		}

protected:
	// Number of players
	float totalPlayers;

	// Index of players names
	unsigned pn[MAX_CLIENTS];

	// Handler for client connections
	virtual void onClientConnected(int clientSocket);

	// Handler for client disconnections
	virtual void onClientDisconnected(int clientSocket);

	// Handler for when a message is received from the client
	virtual void onMessageReceived(int clientSocket, const unsigned char* msg, int length);
};

#endif
