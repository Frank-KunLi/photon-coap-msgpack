#include "application.h"

#include "stdarg.h"
#include "stdbool.h"
#include "string.h"

#include "coapprocessing.h"
#include "endpoints.h"

#define SERIAL_DEBUG

UDP		udp;
STARTUP(WiFi.selectAntenna(ANT_EXTERNAL));


void setup() {
#ifdef SERIAL_DEBUG
    Serial.begin(9600);
#endif

    udp.begin(5683);

    microcoap_setup();
    endpoint_setup();

    coapproc_init(&udp);
#ifdef SERIAL_DEBUG
    coapproc_get()->b_debug = true;
#endif

#ifdef SERIAL_DEBUG
    // Say our IP so one can reach us.
    delay(1000);
    Serial.println(WiFi.localIP());
#endif
}

void loop() {
	// get coap processing object
	coapproc *c = coapproc_get();

	// let it handle a message (if one is available)
	c->ops->handle_udp_coap_message(c);

	// since 0.4.5 UDP is buffered, so we can choose to sleep a bit here
	delay(100);

    // other code goes here
}
