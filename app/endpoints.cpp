#include "application.h"
#include "stdarg.h"
#include "stdbool.h"
#include "string.h"

#include "endpoints.h"
#include "mpack/mpack.h"
#include "microcoap/microcoap.h"


static char light = '0';

const uint16_t rsplen = 1500;
static char rsp[1500] = "";

static int led = D7;

//
void build_wellknowncore_rsp(void);

void endpoint_setup(void)
{
    pinMode(led, OUTPUT);

    //
    build_wellknowncore_rsp();
}

static const microcoap_endpoint_path_t path_well_known_core = {2, {".well-known", "core"}};

static int handle_get_well_known_core(microcoap_rw_buffer_t *scratch,
		const microcoap_packet_t *inpkt, microcoap_packet_t *outpkt,
		uint8_t id_hi, uint8_t id_lo)
{
    return microcoap_make_response(scratch,
    		outpkt,
    		(const uint8_t *)rsp,
			strlen(rsp),
			id_hi,
			id_lo,
			&inpkt->tok,
			COAP_RSPCODE_CONTENT,
			COAP_CONTENTTYPE_APPLICATION_LINKFORMAT
	);
}

static char mpack_buf[256];

static const microcoap_endpoint_path_t path_light = {1, {"light"}};

static int handle_get_light(microcoap_rw_buffer_t *scratch,
		const microcoap_packet_t *inpkt, microcoap_packet_t *outpkt,
		uint8_t id_hi, uint8_t id_lo)
{
	// prepare the message
    mpack_writer_t writer;
    mpack_writer_init(&writer,&mpack_buf[0],sizeof(mpack_buf));

    mpack_start_map(&writer, 1);
    mpack_write_cstr(&writer, "value");
    mpack_start_map(&writer, 2);
    mpack_write_cstr(&writer, "id");
    mpack_write_cstr(&writer, "light");
    mpack_write_cstr(&writer, "v");
    if (light == '0') {
        mpack_write_cstr(&writer, "off");
    }
    if (light == '1') {
        mpack_write_cstr(&writer, "on");
    }
    mpack_finish_map(&writer);
    mpack_finish_map(&writer);

    if (mpack_writer_destroy(&writer) == mpack_ok) {
        return microcoap_make_response(scratch,
        		outpkt,
    			(const uint8_t *)writer.buffer,
    			(size_t)writer.used,
    			id_hi,
    			id_lo,
    			&inpkt->tok,
    			COAP_RSPCODE_CONTENT,
				COAP_CONTENTTYPE_EXPERIMENTAL
    	);
    }

    return microcoap_make_response(scratch,
    		outpkt,
			NULL,
			0,
			id_hi,
			id_lo,
			&inpkt->tok,
			COAP_RSPCODE_INTERNAL_SERVER_ERROR,
			COAP_CONTENTTYPE_NONE
	);
}

static int handle_put_light(microcoap_rw_buffer_t *scratch,
		const microcoap_packet_t *inpkt, microcoap_packet_t *outpkt,
		uint8_t id_hi, uint8_t id_lo)
{
    if (inpkt->payload.len == 0)
        return microcoap_make_response(scratch,
        		outpkt,
				NULL,
				0, id_hi, id_lo,
				&inpkt->tok,
				COAP_RSPCODE_BAD_REQUEST,
				COAP_CONTENTTYPE_TEXT_PLAIN
	);

    mpack_reader_t reader;
    mpack_reader_init_data(&reader,(const char *)(inpkt->payload.p), inpkt->payload.len);

    char buf_mapname[64];

    mpack_expect_map_match(&reader,1);
    mpack_expect_utf8_cstr(&reader,buf_mapname,sizeof(buf_mapname));
    mpack_expect_map_match(&reader,2);

    char buf_id_key[64];
    char buf_id_value[64];
    mpack_expect_utf8_cstr(&reader,buf_id_key,sizeof(buf_id_key));
    mpack_expect_utf8_cstr(&reader,buf_id_value,sizeof(buf_id_value));

    char buf_v_key[64];
    char buf_v_value[64];
    mpack_expect_utf8_cstr(&reader,buf_v_key,sizeof(buf_v_key));
    mpack_expect_utf8_cstr(&reader,buf_v_value,sizeof(buf_v_value));


    mpack_done_map(&reader);
    mpack_done_map(&reader);

    Serial.println(buf_id_key);
    Serial.println(buf_id_value);
    Serial.println(buf_v_key);
    Serial.println(buf_v_value);

    char	buf_error[64];
    mpack_error_t error = mpack_reader_destroy(&reader);
    if ( error != mpack_ok) {
    	strcpy(buf_error,"unable to parse msgpack payload");
    	Serial.printlnf("Msgpack parsing error %d", error);
        return microcoap_make_response(scratch,
        		outpkt,
				(const uint8_t *)&buf_error[0],
				strlen(buf_error),
				id_hi, id_lo,
				&inpkt->tok,
				COAP_RSPCODE_BAD_REQUEST,
				COAP_CONTENTTYPE_TEXT_PLAIN);

    } else {
 /*   	if (strcmp(buf_id_key,"id") != 0) {
    	   	strcpy(buf_error,"first element must be 'id'");
            return microcoap_make_response(scratch,
            		outpkt,
					(const uint8_t *)&buf_error[0],
					strlen(buf_error),
    				0, id_hi, id_lo,
    				&inpkt->tok,
    				COAP_RSPCODE_BAD_REQUEST,
    				COAP_CONTENTTYPE_TEXT_PLAIN
			);

    	}
    	if (strcmp(buf_id_value,"light") != 0) {
    	   	strcpy(buf_error,"'id' must be 'light' for this resource");
            return microcoap_make_response(scratch,
            		outpkt,
					(const uint8_t *)&buf_error[0],
					strlen(buf_error),
    				0, id_hi, id_lo,
    				&inpkt->tok,
    				COAP_RSPCODE_BAD_REQUEST,
    				COAP_CONTENTTYPE_TEXT_PLAIN
			);
    	}
*/

    	if (strcmp(buf_v_value, "on") == 0) {
    		// turn light on
            digitalWrite(led, HIGH);
            light = '1';

            return microcoap_make_response(scratch,
            		outpkt,
    				NULL,
    				0,
    				id_hi,
    				id_lo,
    				&inpkt->tok,
    				COAP_RSPCODE_CHANGED,
    				COAP_CONTENTTYPE_TEXT_PLAIN
    		);
    	}
    	if (strcmp(buf_v_value, "off") == 0) {
    		// turn light on
            digitalWrite(led, LOW);
            light = '0';

            return microcoap_make_response(scratch,
            		outpkt,
    				NULL,
    				0,
    				id_hi,
    				id_lo,
    				&inpkt->tok,
    				COAP_RSPCODE_CHANGED,
    				COAP_CONTENTTYPE_TEXT_PLAIN
    		);
    	}
    }
}

#ifdef __cplusplus
extern "C" {
#endif

extern const microcoap_endpoint_t microcoap_endpoints[] =
{
    {COAP_METHOD_GET, handle_get_well_known_core, &path_well_known_core, "ct=40"},
    {COAP_METHOD_GET, handle_get_light, &path_light, "ct=0"},
    {COAP_METHOD_PUT, handle_put_light, &path_light, NULL},
    {(microcoap_method_t)0, NULL, NULL, NULL}
};

#ifdef __cplusplus
}
#endif


/**
 * iterate all endpoints defined above, create a string describing the endpoints
 */
void build_wellknowncore_rsp(void)
{
    uint16_t len = rsplen;
    const microcoap_endpoint_t *ep = microcoap_endpoints;
    int i;

    len--; // Null-terminated string

    while(NULL != ep->handler)
    {
        if (NULL == ep->core_attr) {
            ep++;
            continue;
        }

        if (0 < strlen(rsp)) {
            strncat(rsp, ",", len);
            len--;
        }

        strncat(rsp, "<", len);
        len--;

        for (i = 0; i < ep->path->count; i++) {
            strncat(rsp, "/", len);
            len--;

            strncat(rsp, ep->path->elems[i], len);
            len -= strlen(ep->path->elems[i]);
        }

        strncat(rsp, ">;", len);
        len -= 2;

        strncat(rsp, ep->core_attr, len);
        len -= strlen(ep->core_attr);

        ep++;
    }
}


