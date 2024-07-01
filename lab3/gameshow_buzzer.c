
#include "gameshow_buzzer.h"

#include "coap_client.h"

#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include "timex.h"
#include "xtimer.h"
#include "net/sntp.h"


void get_iso8601_time(char *buffer, size_t buffer_size) {
    // timex_t now;
    
    struct tm tm_info;
    (void) tm_info;

    // uint64_t microseconds = sntp_get_unix_usec();

    /* --- just for testing  --- */
    uint64_t microseconds = 1719846389000000;
    microseconds += xtimer_now_usec64();
    /* --- just for testing  --- */



    time_t seconds = microseconds / 1000000;
    long remaining_microseconds = microseconds % 1000000;

    
    // Convert seconds since epoch to tm structure
    gmtime_r(&seconds, &tm_info);
    strftime(buffer, buffer_size, "%Y-%m-%dT%H:%M:%S", &tm_info);
    snprintf(buffer + strlen(buffer), buffer_size - strlen(buffer), ".%07ldZ", remaining_microseconds);
  
}

// /*
//  * Response callback.
//  */
// void _resp_handler(const gcoap_request_memo_t *memo, coap_pkt_t* pdu,
//                           const sock_udp_ep_t *remote)
// {

// }

void send_data(const char* uri_base, const char* path, const void* payload, size_t payload_len){
    uint8_t buf[CONFIG_GCOAP_PDU_BUF_SIZE];
    coap_pkt_t pdu;    
    size_t len;

    char uri_buf[128];
 
    sock_udp_ep_t remote;
    
    _uristr2remote(uri_base,&remote,NULL, &uri_buf[0], sizeof(uri_buf));
    
  
    gcoap_req_init(&pdu, buf, CONFIG_GCOAP_PDU_BUF_SIZE, COAP_METHOD_PUT, path);
    coap_opt_add_format(&pdu, COAP_FORMAT_TEXT);
    len = coap_opt_finish(&pdu, COAP_OPT_FINISH_PAYLOAD);

    len += coap_payload_put_bytes(&pdu,payload, payload_len);

    gcoap_req_send(buf,len,&remote,NULL,NULL,0);

}


int get_time(int argc, char **argv){
    (void) argc;
    (void) argv;

    char time_str[26];

    const char uri_base[128] = "coap://[2001:67c:254:b0b2:affe:4000:0:1]:9993";
    char* path = "/b/register";

    char payload[128] = "buzzer1";
    send_data(uri_base,path,(void*) payload,strlen(payload));
    // sntp_sync();

    get_iso8601_time(time_str, sizeof(time_str));
    printf("ISO Time: %s\n",time_str);
    
    return 0;
}