
#ifndef __COAPPROCESSING_H
#define __COAPPROCESSING_H

#include "microcoap/microcoap.h"

typedef struct coapproc_s 					coapproc;
typedef struct coapproc_operations_s 		coapproc_operations;

struct coapproc_s {
	bool	b_debug;
	// Pointer to UDP object
	UDP		*p_udp;
	// Buffer for CoAP messages
	uint8_t packetbuf[2048];
	// Scratch Buffer to CoAP packet creation etc.
	uint8_t scratch_raw[32];
	microcoap_rw_buffer_t scratch_buf = {
			scratch_raw,
			sizeof(scratch_raw)
	};
	coapproc_operations *ops;
};

struct coapproc_operations_s {
	void (*handle_udp_coap_message)(coapproc *obj);
	void (*clear_packet_buf)(coapproc *obj);
};

// Initializes the coap processing struct
coapproc 	*coapproc_init(UDP*);

// Returns instance of coapproc struct
coapproc 	*coapproc_get();

// destroys a coap processing struct (empty, static mem)
void 		coapproc_destroy(coapproc *p);

#endif
