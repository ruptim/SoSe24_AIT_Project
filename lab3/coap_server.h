#ifndef COAP_SERVER_H
#define COAP_SERVER_H


#define MAX_RESOURCES 20
#define CONFIG_URI_MAX 128


#define URI_BASE "/b/"
#define MAX_PUT_PAYLOAD_LEN 11
#define MAX_GET_PAYLOAD_LEN 128

extern coap_resource_t _resources[MAX_RESOURCES];
extern char _resource_uris[MAX_RESOURCES][CONFIG_URI_MAX];
extern gcoap_listener_t _listener;

/**
 * @brief   Registers the CoAP resources exposed in the example app
 *
 * Run this exactly one during startup.
 */
void server_init(gcoap_listener_t *listener);

int get_rgb_values(short int *states, char* payload);

int init_board_periph_resources(void);

void notify_observers(void);


/*
 *   Handler for coap requests to read sensors or drive actuators via SAUL.
 */
ssize_t _riot_board_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);


#endif /* COAP_SERVER_H */