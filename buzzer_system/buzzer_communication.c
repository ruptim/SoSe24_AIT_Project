#include "buzzer_communication.h"

#include "od.h"
#include "uri_parser.h"
#include "ztimer.h"
#include "mutex.h"

#define ENABLE_DEBUG 1
#include "debug.h"
 
char buzzer_heartbeat_thread_stack[THREAD_STACKSIZE_DEFAULT];

const char uri_base[128] = "coap://[2001:67c:254:b0b2:affe:4000:0:1]:9993";
char buzzer_id[MAX_BUZZER_ID_LEN];
bool buzzer_id_received = false;

bool hb_timeout = false;



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






void _data_send_resp_handler(const gcoap_request_memo_t *memo, coap_pkt_t *pdu,
                             const sock_udp_ep_t *remote)
{
    (void)pdu;
    (void)remote;

    /* response timeout or error in response */
    if (memo->state == GCOAP_MEMO_TIMEOUT || memo->state != GCOAP_MEMO_RESP)
    {
        kernel_pid_t *main_thread_pid = (kernel_pid_t *)memo->context;
        msg_t msg;
        msg.content.value = EVENT_NOT_CONNECTED;

        msg_send(&msg, *main_thread_pid);

        DEBUG("CONNECTION LOST\n");
        return;
    }

}


void _heartbeat_resp_handler(const gcoap_request_memo_t *memo, coap_pkt_t *pdu,
                             const sock_udp_ep_t *remote)
{
    (void)pdu;
    (void)remote;
    (void)pdu;

    

    /* response timeout or error in response */
    if (memo->state == GCOAP_MEMO_TIMEOUT || memo->state != GCOAP_MEMO_RESP)
    {

        hb_timeout = true;
    }   

    hb_timeout = false;    
}

void send_heartbeat(void){

    send_data(uri_base, BUZZER_SERVER_HEARTBEAT_URI, (void*) buzzer_id, strlen(buzzer_id), _heartbeat_resp_handler, NULL, true);

}

void *heartbeat_routine(void *args)
{
    (void)args;
    kernel_pid_t *main_thread_pid = (kernel_pid_t *)args;


    

    kernel_pid_t own_pid = thread_getpid();
    ztimer_t sleep_timer;
    
    
    while (!hb_timeout)
    {
        // DEBUG("[INFO] *bum bum*\n");
        hb_timeout = true;

        send_heartbeat();
        
        ztimer_set_wakeup(ZTIMER_MSEC,&sleep_timer, HEARTBEAT_SLEEP_DURATION_MS, own_pid);
        thread_sleep();
        
    }

    if (hb_timeout){
        DEBUG("[INFO] Heartbeat timeouted!\n");
        msg_t msg = {.content=EVENT_NOT_CONNECTED};
        
        msg_send(&msg, *main_thread_pid);
    }

    return 0;
}


void start_heartbeat(kernel_pid_t *main_thread_pid ){

    thread_create(buzzer_heartbeat_thread_stack, sizeof(buzzer_heartbeat_thread_stack),
                  THREAD_PRIORITY_MAIN - 4, 0,
                  heartbeat_routine, (void*) main_thread_pid, "heartbeat_thread");

}

