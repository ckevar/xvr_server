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
static unsigned char pdu_header[] = {255, 88, 86, 82, 95, 48, 50, 255,
                                                 0, 0, 0, 21, // pdu length
                                                 255, 1, 0, 255, 5, // 5 specifies the type, here, it's string
                                                 0, 0, 0, 0}; // message ñength 
float listReqPDU = 98;
float idPDU = 99;
float addListPDU[] = {listReqPDU, 0.f, 1.f};
float rmListPDU[] = {97, 0.f, 1.f};

void CoronaShoot::onClientConnected(int clientSocket) {
	// Send a welcome message to the connected client

	unsigned char pdu[255];
	unsigned ni = rand() % 60;
	size_t len;
	len = badassName[ni].size();

	memcpy(pdu, pdu_header, PDU_BASE_LEN);
	memcpy(pdu + PDU_BASE_LEN, badassName[ni].c_str(), len);
	pdu[PDU_FULL_LEN] += len;
	pdu[PDU_VECTOR_LEN] = len;

	sendToClient(clientSocket, pdu, pdu[PDU_FULL_LEN]);
	totalPlayers += 1;
	pn[(unsigned) totalPlayers - 1] = ni;

	if (totalPlayers > 1) {
		// Broadcast there's a new 
		pdu[PDU_TYPE] = PDU_REAL_T;
		memcpy(pdu + PDU_BASE_LEN, &addListPDU, 12);
		pdu[PDU_FULL_LEN] = PDU_BASE_LEN + 12;
        	pdu[PDU_VECTOR_LEN] = 3;
		broadcastToClients(clientSocket, pdu, pdu[PDU_FULL_LEN]);

		// Broadcaste the new's name
              	memcpy(pdu + PDU_BASE_LEN, badassName[ni].c_str(), len);
                pdu[PDU_TYPE] = PDU_CHAR_T;
		pdu[PDU_FULL_LEN] = PDU_BASE_LEN + len;
                pdu[PDU_VECTOR_LEN] = len;
                broadcastToClients(clientSocket, pdu, pdu[PDU_FULL_LEN]);
	}
}

void CoronaShoot::onClientDisconnected(int clientSocket) {
	unsigned char pdu[255];
	unsigned tmp[MAX_CLIENTS];
	rmListPDU[2] = clientSocket;
	totalPlayers -= 1;

	memcpy(pdu, pdu_header, PDU_BASE_LEN);
	memcpy(pdu + PDU_BASE_LEN, &rmListPDU, 12);
	pdu[PDU_TYPE] = PDU_REAL_T;
	pdu[PDU_FULL_LEN] = PDU_BASE_LEN + 12;
	pdu[PDU_VECTOR_LEN] = 3;
	broadcastToClients(0, pdu, pdu[PDU_FULL_LEN]);
	printf("{debug@onClientDisconnect:} memcpy to temp %d\n", (unsigned)totalPlayers - clientSocket);
	memcpy(tmp, pn + clientSocket + 1, ((unsigned)totalPlayers - clientSocket) * sizeof(unsigned));
	printf("{debug@onClient:} successfully copied\n");
	memcpy(pn + clientSocket, tmp, ((unsigned)totalPlayers - clientSocket) * sizeof(unsigned));
	printf("{debug@onClientDisconnect:} memcpy to mem \n");

}

void CoronaShoot::onMessageReceived(int clientSocket, const unsigned char* msg, int length) {
	float pdu_id;
	unsigned char pdu[255];
	unsigned clientID;

    	if ((length == 29)) {
		memcpy(&pdu_id, msg + PDU_BASE_LEN, 4);
		if (pdu_id == idPDU) {
                	memcpy(&clientID, msg + 25, 4);
                        allocateClient(clientSocket); // service allocation
                        onClientConnected(clientSocket);

    	       	} else if (pdu_id == listReqPDU) {

			memcpy(pdu, msg, length);	// copy original message
			memcpy(pdu + length, &totalPlayers, 4);	// adds a new element
			pdu[PDU_FULL_LEN] += 4;		// update xvr pdu length
			pdu[PDU_VECTOR_LEN] += 1;	// update game pdu length
			sendToClient(clientSocket, pdu, pdu[PDU_FULL_LEN]);

			memcpy(pdu, pdu_header, PDU_BASE_LEN); // to send Text, without game header

			for(int i = 0; i < totalPlayers; i++) {
				size_t len = badassName[pn[i]].size();
				memcpy(pdu + PDU_BASE_LEN, badassName[pn[i]].c_str(), len);
        			pdu[PDU_FULL_LEN] = PDU_BASE_LEN + len;
        			pdu[PDU_VECTOR_LEN] = len;
				sendToClient(clientSocket, pdu, pdu[PDU_FULL_LEN]);
			}
		}
        } else {
		// TODO: check if fd matches clientDI, if not, close the
		// previous fd and reallocate the client to a new fd
		//broadcastToClients(clientSocket, msg, length);
	}

}
