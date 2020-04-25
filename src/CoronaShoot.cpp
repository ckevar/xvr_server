#include "CoronaShoot.h"

#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdlib>

std::string badassName[] = {"Big Papa", "Cobra", "Creep", "Doom", 
"Dragon", "Mad Dog", "Ranger", "Ripley", 
"Skull Casher", "Slasher", "zero", "Iron Heart", 
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

float add_listPDU[] = {LIST_RQ_PDU, 0.f, 1.f};
float rm_listPDU[] 	= {LIST_RM_PDU, 0.f, 1.f};
float set_idPDU[] 	= {0.f, 0.f, 0.f};

void CoronaShoot::buildPDU(unsigned char type, void *msg, unsigned char len) {
	// if real type (float) the size is 4 bytes
	if(type == XVRPDU_REAL_T) {
		memcpy(m_pdu.array, msg, len * sizeof(float));
		*m_pdu.size = XVRPDU_BASE_LEN + len * sizeof(float);

	} 
	// if char type, the size of each char is 1 byte
	else if (type == XVRPDU_CHAR_T) {
		memcpy(m_pdu.array, msg, len);
		*m_pdu.size = XVRPDU_BASE_LEN + len;
	}

	*m_pdu.type = type;
	*m_pdu.length = len;
}

void CoronaShoot::onClientConnected(int clientSocket) {
	set_idPDU[2] = (float) (clientSocket - 3); // 3, due to the server has descriptor 3, so the clients starts in 4;
	buildPDU(XVRPDU_REAL_T, set_idPDU, 3);
	sendToClient(clientSocket, m_pdu.data, *m_pdu.size);
}

void CoronaShoot::nameClient(int clientSocket) {
	// Send a welcome message to the connected client
	unsigned ni = rand() % 59;
	size_t len = badassName[ni].size();
	int clientID = clientSocket - 3;

	*m_players[clientID].id = (float) clientID;
	m_players[clientID].name = (char *) badassName[ni].c_str();
	m_players[clientID].name_len = len;
	totalPlayers += 1;

	printf("\tIndex of names is %d\n", ni);
	printf("\t%s has joined @ socket %d\n", m_players[clientID].name, clientSocket);

	buildPDU(XVRPDU_CHAR_T, m_players[clientID].name, len);
	sendToClient(clientSocket, m_pdu.data, *m_pdu.size);

	if (totalPlayers > 1) {
		add_listPDU[2] = *m_players[clientID].id;
		// Broadcast there's a new player
		buildPDU(XVRPDU_REAL_T, add_listPDU, 3);
		broadcastToClients(clientSocket, m_pdu.data, *m_pdu.size);

		// Broadcaste the new's name
		buildPDU(XVRPDU_CHAR_T, m_players[clientID].name, len);
		broadcastToClients(clientSocket, m_pdu.data, *m_pdu.size);
	}
}

void CoronaShoot::onClientDisconnected(int clientSocket) {
	int clientId = clientSocket - 3;
	totalPlayers -= 1;

	rm_listPDU[2] = *m_players[clientId].id;
	*m_players[clientId].id = -1;

	printf("\t%s has left from socket %d\n", m_players[clientId].name, clientSocket);
	m_players[clientId].name = 0;

	buildPDU(XVRPDU_REAL_T, rm_listPDU, 3);
	broadcastToClients(0, m_pdu.data, *m_pdu.size);
}

void CoronaShoot::onMessageReceived(int clientSocket, const unsigned char* msg, int length) {
	float pdu_id;

	// 29 is the nomial valuen when a vector with two elements are sent, according
	// to XVR NetVarSendFromTCP function
	if ((length == 29)) { 
		// Extract PDU identifier
		memcpy(&pdu_id, msg + XVRPDU_BASE_LEN, sizeof(float));

		// The new-joined player asks for a name
		if (pdu_id == NAME_ME_PDU) {
            //allocateClient(clientSocket); 	// service allocation
            nameClient(clientSocket);
       	}

       	// The new-joined player asks for the liss of players 
       	else if (pdu_id == LIST_RQ_PDU) {

			float tmpIds[totalPlayers];
			int i;
			int j = 0;

			memcpy(m_pdu.data, msg, length);	// copy original message
			
			// Copy the id of the current available players to send them
			for(i = 1; i < MAX_CLIENTS; i++){ // starts at 1, because at 0 is the server
				if(*m_players[i].id > -1) {
					tmpIds[j] = *m_players[i].id;
					j++;
				}
			}

			// Send the ID list to the new-joined player
			memcpy(m_pdu.data + length, tmpIds, 4*totalPlayers);	// appends the player ID list
			*m_pdu.size += 4*totalPlayers;
			*m_pdu.length += totalPlayers;						// update game pdu length
			sendToClient(clientSocket, m_pdu.data, *m_pdu.size);	

			// Send the players' name
			*m_pdu.type = XVRPDU_CHAR_T;
			for(i = 1; i < MAX_CLIENTS; i++) {
				if(*m_players[i].id > -1) {	// send if it's available
					size_t len = m_players[i].name_len;
					memcpy(m_pdu.array, m_players[i].name, len);
					*m_pdu.size = XVRPDU_BASE_LEN + len;
					*m_pdu.length = len;
					sendToClient(clientSocket, m_pdu.data, *m_pdu.size);
				}
			}
			// TODO: broadcast, there's a new guy in town
			allocateClient(clientSocket); 	// service allocation

		}
	// if it's not a header, then broadcast the message to the other players.
	} else {
		broadcastToClients(clientSocket, msg, length);
	}

}
