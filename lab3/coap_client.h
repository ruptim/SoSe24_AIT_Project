#ifndef COAP_CLIENT_H
#define COAP_CLIENT_H

#ifndef CONFIG_URI_MAX
#define CONFIG_URI_MAX      128
#endif
 
#include "net/gcoap.h"
#include "net/sock/util.h"





/**
 * @brief   Shell interface exposing the client side features of gcoap
 * @param   argc    Number of shell arguments (including shell command name)
 * @param   argv    Shell argument values (including shell command name)
 * @return  Exit status of the shell command
 */
int gcoap_cli_cmd(int argc, char **argv);

int _uristr2remote(const char *uri, sock_udp_ep_t *remote, const char **path,
                          char *buf, size_t buf_len);

int _print_usage(char **argv);

ssize_t _send(uint8_t *buf, size_t len, const sock_udp_ep_t *remote,
                     void *ctx, gcoap_socket_type_t tl);

gcoap_socket_type_t _get_tl(const char *uri);

void _resp_handler(const gcoap_request_memo_t *memo, coap_pkt_t* pdu,
                          const sock_udp_ep_t *remote);


void send_data(const char* uri_base, const char* path, const void* payload,
                    size_t payload_len, gcoap_resp_handler_t resp_handler, 
                    void* context);

#endif /* COAP_CLIENT_H */