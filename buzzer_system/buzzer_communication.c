#include "buzzer_communication.h"

#include "od.h"
#include "uri_parser.h"

#define ENABLE_DEBUG 0
#include "debug.h"
 
 

void send_data(const char* uri_base, const char* path, const void* payload, size_t payload_len,
                    gcoap_resp_handler_t resp_handler, void* context, bool confirm){
    uint8_t buf[CONFIG_GCOAP_PDU_BUF_SIZE];
    coap_pkt_t pdu;    
    size_t len;

    char uri_buf[128];
 
    sock_udp_ep_t remote;
    
    _uristr2remote(uri_base,&remote,NULL, &uri_buf[0], sizeof(uri_buf));
    
  
    gcoap_req_init(&pdu, buf, CONFIG_GCOAP_PDU_BUF_SIZE, COAP_METHOD_PUT, path);
    coap_opt_add_format(&pdu, COAP_FORMAT_TEXT);
    if (confirm){
        coap_hdr_set_type(pdu.hdr, COAP_TYPE_CON);
    }
    len = coap_opt_finish(&pdu, COAP_OPT_FINISH_PAYLOAD);

    len += coap_payload_put_bytes(&pdu,payload, payload_len);

    gcoap_req_send(buf,len,&remote,resp_handler,context,0);

}


int _uristr2remote(const char *uri, sock_udp_ep_t *remote, const char **path,
                          char *buf, size_t buf_len)
{
    if (strlen(uri) >= buf_len) {
        DEBUG_PUTS("URI too long");
        return 1;
    }
    uri_parser_result_t urip;
    if (uri_parser_process(&urip, uri, strlen(uri))) {
        DEBUG("'%s' is not a valid URI\n", uri);
        return 1;
    }
    memcpy(buf, urip.host, urip.host_len);
    buf[urip.host_len] = '\0';
    if (urip.port_str_len) {
        strcat(buf, ":");
        strncat(buf, urip.port_str, urip.port_str_len);
        buf[urip.host_len + 1 + urip.port_str_len] = '\0';
    }
    if (sock_udp_name2ep(remote, buf) != 0) {
        DEBUG("Could not resolve address '%s'\n", buf);
        return -1;
    }
    if (remote->port == 0) {
        remote->port = !strncmp("coaps", urip.scheme, 5) ? CONFIG_GCOAPS_PORT : CONFIG_GCOAP_PORT;
    }
    if (path) {
        *path = urip.path;
    }
    strcpy(buf, uri);
    return 0;
}


