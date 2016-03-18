#ifndef MICROCOAP_H
#define MICROCOAP_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAXOPT 16

//http://tools.ietf.org/html/rfc7252#section-3
typedef struct
{
    uint8_t ver;                /* CoAP version number */
    uint8_t t;                  /* CoAP Message Type */
    uint8_t tkl;                /* Token length: indicates length of the Token field */
    uint8_t code;               /* CoAP status code. Can be request (0.xx), success reponse (2.xx), 
                                 * client error response (4.xx), or rever error response (5.xx) 
                                 * For possible values, see http://tools.ietf.org/html/rfc7252#section-12.1 */
    uint8_t id[2];
} microcoap_header_t;

typedef struct
{
    const uint8_t *p;
    size_t len;
} microcoap_buffer_t;

typedef struct
{
    uint8_t *p;
    size_t len;
} microcoap_rw_buffer_t;

typedef struct
{
    uint8_t num;                /* Option number. See http://tools.ietf.org/html/rfc7252#section-5.10 */
    microcoap_buffer_t buf;          /* Option value */
} microcoap_option_t;

typedef struct
{
	microcoap_header_t hdr;          /* Header of the packet */
	microcoap_buffer_t tok;          /* Token value, size as specified by hdr.tkl */
    uint8_t numopts;            /* Number of options */
    microcoap_option_t opts[MAXOPT]; /* Options of the packet. For possible entries see
                                 * http://tools.ietf.org/html/rfc7252#section-5.10 */
    microcoap_buffer_t payload;      /* Payload carried by the packet */
} microcoap_packet_t;

/////////////////////////////////////////

//http://tools.ietf.org/html/rfc7252#section-12.2
typedef enum
{
    COAP_OPTION_IF_MATCH = 1,
    COAP_OPTION_URI_HOST = 3,
    COAP_OPTION_ETAG = 4,
    COAP_OPTION_IF_NONE_MATCH = 5,
    COAP_OPTION_OBSERVE = 6,
    COAP_OPTION_URI_PORT = 7,
    COAP_OPTION_LOCATION_PATH = 8,
    COAP_OPTION_URI_PATH = 11,
    COAP_OPTION_CONTENT_FORMAT = 12,
    COAP_OPTION_MAX_AGE = 14,
    COAP_OPTION_URI_QUERY = 15,
    COAP_OPTION_ACCEPT = 17,
    COAP_OPTION_LOCATION_QUERY = 20,
    COAP_OPTION_PROXY_URI = 35,
    COAP_OPTION_PROXY_SCHEME = 39
} microcoap_option_num_t;

//http://tools.ietf.org/html/rfc7252#section-12.1.1
typedef enum
{
    COAP_METHOD_GET = 1,
    COAP_METHOD_POST = 2,
    COAP_METHOD_PUT = 3,
    COAP_METHOD_DELETE = 4
} microcoap_method_t;

//http://tools.ietf.org/html/rfc7252#section-12.1.1
typedef enum
{
    COAP_TYPE_CON = 0,
    COAP_TYPE_NONCON = 1,
    COAP_TYPE_ACK = 2,
    COAP_TYPE_RESET = 3
} microcoap_msgtype_t;

//http://tools.ietf.org/html/rfc7252#section-5.2
//http://tools.ietf.org/html/rfc7252#section-12.1.2
#define MAKE_RSPCODE(clas, det) ((clas << 5) | (det))
typedef enum
{
    COAP_RSPCODE_CONTENT = MAKE_RSPCODE(2, 5),
    COAP_RSPCODE_NOT_FOUND = MAKE_RSPCODE(4, 4),
    COAP_RSPCODE_BAD_REQUEST = MAKE_RSPCODE(4, 0),
    COAP_RSPCODE_CHANGED = MAKE_RSPCODE(2, 4),
	COAP_RSPCODE_INTERNAL_SERVER_ERROR = MAKE_RSPCODE(5,0)
} microcoap_responsecode_t;

//http://tools.ietf.org/html/rfc7252#section-12.3
typedef enum
{
    COAP_CONTENTTYPE_NONE = -1, // bodge to allow us not to send option block
    COAP_CONTENTTYPE_TEXT_PLAIN = 0,
    COAP_CONTENTTYPE_APPLICATION_LINKFORMAT = 40,
    COAP_CONTENTTYPE_APPLICATION_XML = 41,
    COAP_CONTENTTYPE_APPLICATION_OCTECT_STREAM = 42,
    COAP_CONTENTTYPE_APPLICATION_EXI = 47,
    COAP_CONTENTTYPE_APPLICATION_JSON = 50,
    COAP_CONTENTTYPE_EXPERIMENTAL = 201,
} microcoap_content_type_t;

///////////////////////

typedef enum
{
    COAP_ERR_NONE = 0,
    COAP_ERR_HEADER_TOO_SHORT = 1,
    COAP_ERR_VERSION_NOT_1 = 2,
    COAP_ERR_TOKEN_TOO_SHORT = 3,
    COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER = 4,
    COAP_ERR_OPTION_TOO_SHORT = 5,
    COAP_ERR_OPTION_OVERRUNS_PACKET = 6,
    COAP_ERR_OPTION_TOO_BIG = 7,
    COAP_ERR_OPTION_LEN_INVALID = 8,
    COAP_ERR_BUFFER_TOO_SMALL = 9,
    COAP_ERR_UNSUPPORTED = 10,
    COAP_ERR_OPTION_DELTA_INVALID = 11,
} microcoap_error_t;

///////////////////////

typedef int (*microcoap_endpoint_func)(microcoap_rw_buffer_t *scratch, const microcoap_packet_t *inpkt, microcoap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo);
#define MAX_SEGMENTS 2  // 2 = /foo/bar, 3 = /foo/bar/baz
typedef struct
{
    int count;
    const char *elems[MAX_SEGMENTS];
} microcoap_endpoint_path_t;

typedef struct
{
	microcoap_method_t method;               /* (i.e. POST, PUT or GET) */
	microcoap_endpoint_func handler;         /* callback function which handles this
                                         * type of endpoint (and calls 
                                         * coap_make_response() at some point) */
    const microcoap_endpoint_path_t *path;   /* path towards a resource (i.e. foo/bar/) */
    const char *core_attr;              /* the 'ct' attribute, as defined in RFC7252, section 7.2.1.:
                                         * "The Content-Format code "ct" attribute 
                                         * provides a hint about the 
                                         * Content-Formats this resource returns." 
                                         * (Section 12.3. lists possible ct values.) */
} microcoap_endpoint_t;


///////////////////////
void microcoap_dumpPacket(microcoap_packet_t *pkt);
int microcoap_parse(microcoap_packet_t *pkt, const uint8_t *buf, size_t buflen);
int microcoap_buffer_to_string(char *strbuf, size_t strbuflen, const microcoap_buffer_t *buf);
const microcoap_option_t *microcoap_findOptions(const microcoap_packet_t *pkt, uint8_t num, uint8_t *count);
int microcoap_build(uint8_t *buf, size_t *buflen, const microcoap_packet_t *pkt);
void microcoap_dump(const uint8_t *buf, size_t buflen, bool bare);
int microcoap_make_response(microcoap_rw_buffer_t *scratch, microcoap_packet_t *pkt, const uint8_t *content, size_t content_len, uint8_t msgid_hi, uint8_t msgid_lo, const microcoap_buffer_t* tok, microcoap_responsecode_t rspcode, microcoap_content_type_t content_type);
int microcoap_handle_req(microcoap_rw_buffer_t *scratch, const microcoap_packet_t *inpkt, microcoap_packet_t *outpkt);
void microcoap_option_nibble(uint32_t value, uint8_t *nibble);
void microcoap_setup(void);
//void endpoint_setup(void);

#ifdef __cplusplus
}
#endif

#endif