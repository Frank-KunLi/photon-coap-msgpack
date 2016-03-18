#include "application.h"

#include "stdarg.h"
#include "stdbool.h"
#include "string.h"

#include "coapprocessing.h"

//
void _handle_udp_coap_message(coapproc *obj) {

		int sz;
	    int rc;
	    microcoap_packet_t pkt;
	    int i;

	    if ((sz = obj->p_udp->parsePacket()) > 0)
	    {
	    	obj->p_udp->read(obj->packetbuf, sizeof(obj->packetbuf));

	    	if (obj->b_debug) {
				for (i=0;i<sz;i++)
				{
					Serial.print(obj->packetbuf[i], HEX);
					Serial.print(" ");
				}
				Serial.println("");
	    	}

	        if (0 != (rc = microcoap_parse(&pkt, obj->packetbuf, sz)))
	        {
		    	if (obj->b_debug) {
		    		Serial.print("Bad packet rc=");
		    		Serial.println(rc, DEC);
		    	}
	        }
	        else
	        {
		    	if (obj->b_debug) {
		    		Serial.println("Processing CoAP message");
		    	}

	        	size_t rsplen = sizeof(obj->packetbuf);
	            microcoap_packet_t rsppkt;
	            microcoap_handle_req(&obj->scratch_buf, &pkt, &rsppkt);

	            memset(obj->packetbuf, 0, sizeof(obj->packetbuf));
	            if (0 != (rc = microcoap_build(obj->packetbuf, &rsplen, &rsppkt)))
	            {
	    	    	if (obj->b_debug) {
	    	    		Serial.print("coap_build failed rc=");
	    	    		Serial.println(rc, DEC);
	    	    	}
	            }
	            else
	            {
	            	obj->p_udp->beginPacket(obj->p_udp->remoteIP(), obj->p_udp->remotePort());
	                uint8_t *buf = obj->packetbuf;
	                while(rsplen--)
	                	obj->p_udp->write(*buf++);
	                obj->p_udp->endPacket();
	            }
	        }
	    }
}

void clear_packet_buf(coapproc *obj) {
    memset(obj->packetbuf, 0, sizeof(obj->packetbuf));
}

static coapproc		coapproc_obj;
static coapproc_operations		coapproc_operations_obj;

coapproc 	*coapproc_init(UDP *p_udp) {
	coapproc *obj = &coapproc_obj;

	coapproc_obj.p_udp = p_udp;
	obj->ops = &coapproc_operations_obj;
	obj->ops->handle_udp_coap_message = _handle_udp_coap_message;

	return obj;
}
coapproc 	*coapproc_get() {
	coapproc *obj = &coapproc_obj;
	return obj;
}
void 		coapproc_destroy(coapproc *p) {
	;
}
