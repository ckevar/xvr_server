#ifndef CORONA_SHOOT_H
#define CORONA_SHOOT_H

#include "TcpListener.h"
#include "XVRPDU.h"

#include <cstring>

// Global Variables
extern float listReqPDU, idPDU;

struct PLAYER {
	size_t name_len;
	char *name;
	float *id;
};

class CoronaShoot : public TcpListener {
public:

	CoronaShoot(const char* ipAddress, int port) :
		TcpListener(ipAddress, port) {
			totalPlayers = 0;
			memcpy(m_pdu_array, pdu_header, PDU_BASE_LEN);

			m_pdu.data = m_pdu_array;
			m_pdu.type = m_pdu_array + PDU_TYPE;
			m_pdu.size_byte = m_pdu_array + PDU_FULL_LEN;
			m_pdu.size_array = m_pdu_array + PDU_VECTOR_LEN;
			m_pdu.array = m_pdu_array + PDU_BASE_LEN;

			for(int i = 0; i < MAX_CLIENTS; i++) {
				id_players_array[i + 1] = -1;
				m_players[i + 1].id = id_players_array + i + 1;
			}
		}

protected:
	// Number of players
	unsigned totalPlayers;

	// Players List
	struct PLAYER m_players[MAX_CLIENTS + 1];
	float id_players_array[MAX_CLIENTS + 1];

	// game PDU array
	unsigned char m_pdu_array[255];

	// Handler for PDU
	struct XVRPDU m_pdu;

	// to build game XVR PDU
	void buildPDU(unsigned char type, void *msg, unsigned char len);

	// Name client
	void nameClient(int clientSocket);

	// Handler for client connections
	virtual void onClientConnected(int clientSocket);

	// Handler for client disconnections
	virtual void onClientDisconnected(int clientSocket);

	// Handler for when a message is received from the client
	virtual void onMessageReceived(int clientSocket, const unsigned char* msg, int length);
};

#endif
