#ifndef XVR_PDU_H
#define XVR_PDU_H

#define XVRPDU_SIZE			11 // XVR total pdu length
#define XVRPDU_MSG_LEN		20 // XVR total vector length
#define XVRPDU_BASE_LEN		21 // XVR position where it starts
#define XVRPDU_TYPE			16 // XVR type 5:char, 8 float32_t
#define XVRPDU_REAL_T		8  // XVR real float32_t type
#define XVRPDU_CHAR_T		5  // XVR char type

static unsigned char XVRpdu_base[] = {
	255, 88, 86, 82, 95, 48, 50, 255, 	// XVR NetVarSendFromTCP / XVR NetVarReceiveFromTCP header
	0, 0, 0, 21, 						// XVR PDU length in bytes
	255, 1, 0, 255, 					// XVR PDU Uknonwn 
	5, 									// XVR PDU specifies the type, 5: string, 8: vector of real
	0, 0, 0, 0							// Application message length in number of elements
}; 						

struct XVRPDU {
	unsigned char *data;
	unsigned char *type;
	unsigned char *size;
	unsigned char *length;
	unsigned char *array;
};

#endif
