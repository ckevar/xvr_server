#include "CoronaShoot.h"

#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdlib>

std::string badassName[] = {"Big Papa", "Cobra", "Creep", "Doom", 
"Dragon", "Mad Dog", "Ranger", "Ripley", 
"Skull crasher", "Slasher", "zero", "Iron Heart", 
"Golem", "Rex", "Cyclops", "Leviathan", 
"Fury", "Terminator", "Mad Max", "Darko", 
"the Butcher", "Maximu", "Mr. Blonde", "Thor", 
"Mr. Pool", "Hannibal", "Leonidas", "Jesse James", 
"Atilla", "Annihilator", "the Finisher", "Hell-Raiser", 
"Exterminator", "Majesty", "Mirage", "Belladonna", 
"Calypso", "Femme Fatale", "Tequila", "Velvet", 
"Pearl Heart", "Caprice", "Night terror", "Miss Fortune", 
"Miss Poison", "Kiddo", "Padme", "Arya", 
"Ramona", "Elektra", "Hypernova", "Snake Eye", 
"Red Dog", "Black Cat", "Firebringer", "Eternity", 
"Maniac", "Bitten", "X-Treme"};

float listReqPDU = 98;
float idPDU = 99;
float addListPDU[] = {listReqPDU, 0.f, 1.f};
float rmListPDU[] = {97, 0.f, 1.f};
float giveIDPDU[] = {100, 0.f, 0.f};

void CoronaShoot::buildPDU(unsigned char type, void *msg, unsigned char len) {
	if(type == PDU_REAL_T) {
		memcpy(m_pdu.array, msg, len*4);
		*m_pdu.size_byte = PDU_BASE_LEN + len * 4;
	} else if (type == PDU_CHAR_T) {
		memcpy(m_pdu.array, msg, len);
		*m_pdu.size_byte = PDU_BASE_LEN + len;
	}
	*m_pdu.type = type;
	*m_pdu.size_array = len;
}

void CoronaShoot::onClientConnected(int clientSocket) {
	giveIDPDU[2] = (float) (clientSocket - 3); // 3, due to the server has descriptor 3, so the clients starts in 4;
	buildPDU(PDU_REAL_T, giveIDPDU, 3);
	sendToClient(clientSocket, m_pdu.data, *m_pdu.size_byte);
}

void CoronaShoot::nameClient(int clientSocket) {
	// Send a welcome message to the connected client
	unsigned ni = rand() % 60;
	size_t len = badassName[ni].size();
	int clientID = clientSocket - 3;

	*m_players[clientID].id = (float) clientID;
	m_players[clientID].name = (char *) badassName[ni].c_str();
	m_players[clientID].name_len = len;
	totalPlayers += 1;

	printf("%s has joined @ socket %d\n", m_players[clientID].name, clientSocket);

	buildPDU(PDU_CHAR_T, m_players[clientID].name, len);
	sendToClient(clientSocket, m_pdu.data, *m_pdu.size_byte);

	if (totalPlayers > 1) {
		addListPDU[2] = *m_players[clientID].id;
		// Broadcast there's a new player
		buildPDU(PDU_REAL_T, &addListPDU, 3);
		broadcastToClients(clientSocket, m_pdu.data, *m_pdu.size_byte);

		// Broadcaste the new's name
		buildPDU(PDU_CHAR_T, m_players[clientID].name, len);
		broadcastToClients(clientSocket, m_pdu.data, *m_pdu.size_byte);
	}
}

void CoronaShoot::onClientDisconnected(int clientSocket) {
	int clientId = clientSocket - 3;
	totalPlayers -= 1;

	rmListPDU[2] = *m_players[clientId].id;
	*m_players[clientId].id = -1;

	printf("%s has left from socket %d\n", m_players[clientId].name, clientSocket);
	m_players[clientId].name = 0;

	buildPDU(PDU_REAL_T, rmListPDU, 3);
	broadcastToClients(0, m_pdu.data, *m_pdu.size_byte);
}

void CoronaShoot::onMessageReceived(int clientSocket, const unsigned char* msg, int length) {
	float pdu_id;
	unsigned clientID;

    	if ((length == 29)) {
		memcpy(&pdu_id, msg + PDU_BASE_LEN, 4);
		if (pdu_id == idPDU) {
                	memcpy(&clientID, msg + 25, 4);
                        allocateClient(clientSocket); // service allocation
                        nameClient(clientSocket);

    	       	} else if (pdu_id == listReqPDU) {

			float tmpIds[totalPlayers];
			memcpy(m_pdu.data, msg, length);	// copy original message
			int j = 0;
			for(int i = 1; i < MAX_CLIENTS; i++){
				if(*m_players[i].id > -1) {
					tmpIds[j] = *m_players[i].id;
					j++;
				}
			}
			memcpy(m_pdu.data + length, tmpIds, 4*totalPlayers);	// adds a new element
			*m_pdu.size_byte += 4*totalPlayers;
			*m_pdu.size_array += totalPlayers;			// update game pdu length
			sendToClient(clientSocket, m_pdu.data, *m_pdu.size_byte);

			*m_pdu.type = PDU_CHAR_T;
			for(int i = 1; i < MAX_CLIENTS; i++) {
				if(*m_players[i].id > -1) {
					size_t len = m_players[i].name_len;
					memcpy(m_pdu.array, m_players[i].name, len);
					*m_pdu.size_byte = PDU_BASE_LEN + len;
					*m_pdu.size_array = len;
					sendToClient(clientSocket, m_pdu.data, *m_pdu.size_byte);
				}
			}
		}
        } else {
		// TODO: check if fd matches clientDI, if not, close the
		// previous fd and reallocate the client to a new fd
		//broadcastToClients(clientSocket, msg, length);
	}

}
