#ifndef BUZZER_COMMUNICATION_H
#define BUZZER_COMMUNICATION_H

#ifndef CONFIG_URI_MAX
#define CONFIG_URI_MAX      128
#endif 

#define MAX_PUT_PAYLOAD_LEN 64
 
#include "net/gcoap.h"
#include "net/sock/util.h" 


void send_data(const char* uri_base, const char* path, const void* payload, size_t payload_len,
                    gcoap_resp_handler_t resp_handler, void* context, bool confirm);

int _uristr2remote(const char *uri, sock_udp_ep_t *remote, const char **path,
                          char *buf, size_t buf_len);

#endif /* BUZZER_COMMUNICATION_H */