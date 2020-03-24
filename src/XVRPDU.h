#ifndef XVR_PDU_H
#define XVR_PDU_H

#define PDU_FULL_LEN    11 // XVR total pdu length
#define PDU_VECTOR_LEN  20 // XVR total vector length
#define PDU_BASE_LEN    21 // XVR position where it starts
#define PDU_TYPE        16 // XVR type 5:char, 8 float32_t
#define PDU_REAL_T      8  // XVR real float32_t type
#define PDU_CHAR_T      5  // XVR char type

static unsigned char pdu_header[] = {255, 88, 86, 82, 95, 48, 50, 255,
                                                 0, 0, 0, 21, // pdu length
                                                 255, 1, 0, 255, 5, // 5 specifies the type, here, it's string
                                                 0, 0, 0, 0}; // message ñength

struct XVRPDU {
	unsigned char *data;
        unsigned char *type;
        unsigned char *size_byte;
        unsigned char *size_array;
        unsigned char *array;
};

#endif
