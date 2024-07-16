#include "buzzer_communication.h"

#include "od.h"
#include "uri_parser.h"
#include "ztimer.h"
#include "mutex.h"

#include "net/gnrc.h"
#include "net/ipv6.h"
#include "net/gnrc/ipv6/nib.h"

#define ENABLE_DEBUG 1
#include "debug.h"
 
char buzzer_heartbeat_thread_stack[THREAD_STACKSIZE_DEFAULT];


mutex_t coap_send_mutex = MUTEX_INIT;

// const char uri_base[128] = "coap://[fe80::d07c:fc5d:2474:4dba]:9993";
// char uri_base[128] = "coap://[2001:67c:254:b0b2:affe:4000:0:1]:9993";
char uri_base[128] = "";

char sntp_server[128];
char buzzer_id[MAX_BUZZER_ID_LEN];
int buzzer_id_num = -1;

bool buzzer_id_received = false;

bool hb_timeout = false;
bool connection_lost = false;

int get_rd_address(void){

    gnrc_ipv6_nib_init();

    gnrc_ipv6_nib_abr_t entry;
    void *state = NULL;
    (void) state;

    while (gnrc_ipv6_nib_abr_iter(&state, &entry)) {
        break;
    }

    char rd_address[IPV6_ADDR_MAX_STR_LEN+3] = "";
    char addr_str[IPV6_ADDR_MAX_STR_LEN];

    ipv6_addr_to_str(addr_str, &entry.addr, sizeof(addr_str));
    snprintf(rd_address,IPV6_ADDR_MAX_STR_LEN+3,"[%s]",addr_str);

    snprintf(uri_base,sizeof(uri_base),"coap://[%s]:9993",addr_str);
    snprintf(sntp_server,sizeof(sntp_server),"[%s]:123",addr_str);

    DEBUG("URI BASE: %s\n",uri_base);
    return 0;
    
}



void set_connection_lost(bool status){
    connection_lost = status;
}


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

    // mutex_lock(&coap_send_mutex);
    int r = gcoap_req_send(buf,len,&remote,resp_handler,context, GCOAP_SOCKET_TYPE_UDP);
    if(r == 0){
        DEBUG("REQ SEND failed\n");
    }
    // mutex_unlock(&coap_send_mutex);

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
    (void)memo;


    /* response timeout or error in response */
    if (memo->state == GCOAP_MEMO_TIMEOUT || memo->state != GCOAP_MEMO_RESP)
    {
        // kernel_pid_t *main_thread_pid = (kernel_pid_t *)memo->context;
        // msg_t msg;
        // msg.content.value = EVENT_NOT_CONNECTED;
        // set_connection_lost(true);

        // msg_send(&msg, *main_thread_pid);

        DEBUG("MSG Request timeouted\n");
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
        return;
    }   

    unsigned int code_flag = pdu->hdr->code;


    /* if pairing request was not accepted, set timeout */
    if(code_flag == COAP_CODE_FORBIDDEN){
         hb_timeout = true;
         return;
    }

    hb_timeout = false;    
}

void send_heartbeat(void){

    send_data(uri_base, BUZZER_SERVER_HEARTBEAT_URI, (void*) buzzer_id, strlen(buzzer_id), _heartbeat_resp_handler, NULL, true);

}

void *heartbeat_routine(void *args)
{
    DEBUG("[INFO] Heartbeat started\n");
    (void)args;
    kernel_pid_t *main_thread_pid = (kernel_pid_t *)args;

    hb_timeout = false;
    

    kernel_pid_t own_pid = thread_getpid();
    ztimer_t sleep_timer;
    
    printf("[DB]: %d, %d\n", hb_timeout,connection_lost);
    while (!hb_timeout && !connection_lost)
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
    set_connection_lost(false);
    thread_create(buzzer_heartbeat_thread_stack, sizeof(buzzer_heartbeat_thread_stack),
                  THREAD_PRIORITY_MAIN - 4, 0,
                  heartbeat_routine, (void*) main_thread_pid, "heartbeat_thread");

}

