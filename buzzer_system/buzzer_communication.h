#ifndef BUZZER_COMMUNICATION_H
#define BUZZER_COMMUNICATION_H

 
#include "net/gcoap.h"
#include "net/sock/util.h" 
#include "thread.h" 

#include "defines.h"


#ifndef CONFIG_URI_MAX
#define CONFIG_URI_MAX      128
#endif 

#define MAX_PUT_PAYLOAD_LEN 64

extern char sntp_server[128];
extern char uri_base[128];
extern char buzzer_id[MAX_BUZZER_ID_LEN];
extern int buzzer_id_num;
extern bool buzzer_id_received;


void send_data(const char* uri_base, const char* path, const void* payload, size_t payload_len,
                    gcoap_resp_handler_t resp_handler, void* context, bool confirm);

int _uristr2remote(const char *uri, sock_udp_ep_t *remote, const char **path,
                          char *buf, size_t buf_len);

void _data_send_resp_handler(const gcoap_request_memo_t *memo, coap_pkt_t *pdu,
                             const sock_udp_ep_t *remote);

void _heartbeat_resp_handler(const gcoap_request_memo_t *memo, coap_pkt_t *pdu,
                             const sock_udp_ep_t *remote);

void send_heartbeat(void);

int get_rd_address(void);

void *heartbeat_routine(void *args);

void start_heartbeat(kernel_pid_t *main_thread_pid);

void set_connection_lost(bool status);

#endif /* BUZZER_COMMUNICATION_H */