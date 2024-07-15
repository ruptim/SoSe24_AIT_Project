
#include "gameshow_buzzer.h"

#include "buzzer_communication.h"
#include "defines.h"

#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include "timex.h"
#include "xtimer.h"
#include "tm.h"
#include "net/sntp.h"

#include "periph/gpio.h"
#include "ztimer.h"
#include "mutex.h"

#define MAX_RESOURCES 10

#define DEBOUNCE_DELAY_MS 300
#define BUZZER_ACTIVE_STATE 0
#define BUZZER_PASSIV_STATE 1
#define BUZZER_PAIRING_MODE_BLINK_DELAY_MS 200
#define BUZZER_NOT_CONNECTED_BLINK_DELAY_MS 1000
#define BUZZER_NORMAL_MODE_LED_STATE 1

#define MAX_SNTP_ATTEMPS 5
#define MAX_PAIRING_ATTEMPS 1

#define PAIRING_MODE_PRESS_DELAY_MS 2000
#define RE_PAIRING_MODE_PRESS_DELAY_MS 5000
#define PAIRING_MODE_DEBOUNCE_TIME_MS 10
#define PAIRING_MODE_TIMEOUT_TIME_MS 5000
#define TIME_ZONE_OFFSET_HOUR 2

// #define BUZZER_SWITCH_PIN
// #define BUZZER_LED_PIN PORT_E, 18

coap_resource_t _buzzer_resources[MAX_RESOURCES];
char _buzzer_resource_uris[MAX_RESOURCES][CONFIG_URI_MAX];
uint8_t buzzer_resource_count = 0;
gcoap_listener_t _buzzer_listener;

ztimer_now_t btn_debounce_ts;
ztimer_now_t btn_long_press_debounce_ts;
ztimer_t timer;
mutex_t led1_mutex = MUTEX_INIT;

bool init_done = false;
bool buzzer_paired = false;
bool buzzer_not_connected_mode = true;
bool buzzer_pairing_mode = false;
bool buzzer_pairing_mode_timeout = false;
bool buzzer_locked = false;
bool    switch_activated = false;

mutex_t buzzer_mutex = MUTEX_INIT;
ztimer_t long_press_timer;
#ifdef BOARD_FEATHER
unsigned int btn = GPIO_PIN(0, 8);
unsigned int led1 = GPIO_PIN(0, 6);
#else 
unsigned int btn = GPIO_PIN(PORT_A, 2);
unsigned int led1 = GPIO_PIN(PORT_A, 1);
#endif

char buzzer_pairing_thread_stack[THREAD_STACKSIZE_DEFAULT];
char buzzer_blink_thread_stack[THREAD_STACKSIZE_DEFAULT];
char buzzer_pairing_blink_thread_stack[THREAD_STACKSIZE_DEFAULT];



#define ENABLE_DEBUG 1
#include "debug.h"

enum BLINK_MODES
{
    BLINK_NOT_CONN,
    BLINK_PAIR_MODE,
};



void lock_buzzer(void)
{
    mutex_lock(&buzzer_mutex);
    buzzer_locked = true;
    gpio_write(led1, !BUZZER_NORMAL_MODE_LED_STATE);
    mutex_unlock(&buzzer_mutex);
}

void unlock_buzzer(void)
{
    mutex_lock(&buzzer_mutex);
    buzzer_locked = false;
    gpio_write(led1, BUZZER_NORMAL_MODE_LED_STATE);
    mutex_unlock(&buzzer_mutex);
}

void enable_normal_mode(void)
{
    unlock_buzzer();
}

void enable_not_connected_mode(void)
{
    set_connection_status(false);
    unlock_buzzer();
    start_not_conn_blink_thread();
}

void disable_normal_mode(void)
{
    lock_buzzer();
}

void set_connection_status(bool connected)
{
    buzzer_paired = connected;
}

void start_heartbeat_routine(kernel_pid_t *main_thread_pid){
    start_heartbeat(main_thread_pid);
}

void send_buzzer_pressed(kernel_pid_t *main_thread_pid)
{                       
    char time_str[TIME_STRING_LEN];
    get_iso8601_time(time_str, sizeof(time_str));

    char payload[MAX_PUT_PAYLOAD_LEN] = "";
    strcat(payload, buzzer_id);
    strcat(payload, ",");
    strcat(payload, time_str);

    DEBUG("SENDING: %s\n", payload);
    (void) main_thread_pid;

    send_data(uri_base, BUZZER_SERVER_PRESSED_URI, (void *)payload, strlen(payload), _data_send_resp_handler, (void *)main_thread_pid, false);
}



void pair_resp_handler(const gcoap_request_memo_t *memo, coap_pkt_t *pdu,
                       const sock_udp_ep_t *remote)
{
    (void)pdu;
    (void)remote;

    kernel_pid_t *pairing_thread_pid = (kernel_pid_t *)memo->context;

    /* response timeout or error in response */
    if (memo->state == GCOAP_MEMO_TIMEOUT || memo->state != GCOAP_MEMO_RESP)
    {
        buzzer_pairing_mode_timeout = true;
        thread_wakeup(*pairing_thread_pid);
        DEBUG("PAIR TIMEOUT\n");
        return;
    }

    unsigned int code_flag = coap_get_code_class(pdu);

    char payload[MAX_PUT_PAYLOAD_LEN];
    memcpy(payload, (char *)pdu->payload, pdu->payload_len);
    if (pdu->payload_len < MAX_PUT_PAYLOAD_LEN){
        payload[pdu->payload_len] = '\0'; // append '\0' for when payload is single character

    }
    DEBUG("[INFO] RESPONSE CODE %d: %s, %d, %d\n",code_flag,payload,strlen(payload),pdu->payload_len);

   if(strlen(payload) < 3){
        if (strcmp(payload, BUZZER_SERVER_RE_REGISTER_OK) == 0)
        {
            DEBUG("[INFO] Re-registere success\n");
            // all good; given name is still registered
        }
      
    }
    else
    {
        DEBUG("[INFO] Registered name: %s\n",payload);
        memcpy(buzzer_id, payload, MAX_BUZZER_ID_LEN);
        buzzer_id_received = true;
    }

    DEBUG("BUZZER PAIRED\n");

    buzzer_pairing_mode = false;

    thread_wakeup(*pairing_thread_pid);
}

//TODO: HEARTBEAT TODO: 



void send_pair_request(kernel_pid_t *pairing_thread_pid)
{
    
    char payload[MAX_PUT_PAYLOAD_LEN] = "";

    if(buzzer_id_received){
        memcpy(payload,buzzer_id,strlen(buzzer_id));
    }

    send_data(uri_base, BUZZER_SERVER_REGISTER_URI, (void *)payload, strlen(payload), pair_resp_handler, (void *)pairing_thread_pid, true);
}


void *pairing_blink_routine(void *args)
{
    (void)args;

    DEBUG("START PAIRING BLINK\n");

    while (buzzer_pairing_mode)
    {
        gpio_toggle(led1);
        ztimer_sleep(ZTIMER_MSEC, BUZZER_PAIRING_MODE_BLINK_DELAY_MS);
    }
    DEBUG("STOPPED PAIRING BLINK\n");

    return 0;
}

void *not_connected_blink_routine(void *args)
{
    (void)args;

    DEBUG("START NOT CONNECTED BLINK\n");

    while (buzzer_not_connected_mode)
    {
        gpio_toggle(led1);
        ztimer_sleep(ZTIMER_MSEC, BUZZER_NOT_CONNECTED_BLINK_DELAY_MS);
    }

    DEBUG("STOPPED NOT CONNECTED BLINK\n");

    return 0;
}

void start_not_conn_blink_thread(void)
{
    buzzer_not_connected_mode = true;
    thread_create(buzzer_blink_thread_stack, sizeof(buzzer_blink_thread_stack),
                  THREAD_PRIORITY_MAIN - 2, THREAD_CREATE_STACKTEST,
                  not_connected_blink_routine, NULL, "not_conn_pairing_thread");
}

void *pairing_mode_routine(void *args)
{
    (void)args;
    kernel_pid_t *main_thread_pid = (kernel_pid_t *)args;
    msg_t msg;

    buzzer_pairing_mode = true;

    kernel_pid_t own_pid = thread_getpid();

    thread_create(buzzer_pairing_blink_thread_stack, sizeof(buzzer_pairing_blink_thread_stack),
                  THREAD_PRIORITY_MAIN - 3, THREAD_CREATE_STACKTEST,
                  pairing_blink_routine, NULL, "blink_thread");

    int attemps = MAX_PAIRING_ATTEMPS;
    while (buzzer_pairing_mode && attemps != 0)
    {
        DEBUG("PAIRING MODE!!\n");
        buzzer_pairing_mode_timeout = false;
        send_pair_request(&own_pid);

        thread_sleep();
        attemps--;
    }

    if (buzzer_pairing_mode_timeout)
    {
        buzzer_pairing_mode = false;
        msg.content.value = EVENT_NOT_CONNECTED;
    }
    else
    {
        msg.content.value = EVENT_CONNECTED;
    }
    msg_send(&msg, *main_thread_pid);

    return 0;
}

void start_pairing_routine(kernel_pid_t *main_thread_pid)
{
    buzzer_not_connected_mode = false;

    thread_create(buzzer_pairing_thread_stack, sizeof(buzzer_pairing_thread_stack),
                  THREAD_PRIORITY_MAIN - 2, 0,
                  pairing_mode_routine, (void *)main_thread_pid, "pairing_thread");
}

void long_press_callback(void *args)
{
    (void)args;
    kernel_pid_t *main_thread_pid = (kernel_pid_t *)args;
    msg_t msg;
    msg.content.value = EVENT_START_PAIRING;

    DEBUG("LONG PRESS REGISTERED!\n");

    msg_send(&msg, *main_thread_pid);
}

static void btn_callback(void *args)
{
    (void)args;
    kernel_pid_t *main_thread_pid = (kernel_pid_t *)args;
    msg_t msg;
    msg.content.value = EVENT_BUZZER_PRESSED;

    ztimer_acquire(ZTIMER_MSEC);
    ztimer_now_t cur_ts = ztimer_now(ZTIMER_MSEC);
    ztimer_release(ZTIMER_MSEC);

    int state = gpio_read(btn);


    if (!buzzer_pairing_mode)
    {
        /* if HASN'T been pressed yet but pressed now and outside of the debounce window */
        if ((!switch_activated && state == BUZZER_ACTIVE_STATE) && (cur_ts - btn_long_press_debounce_ts >= PAIRING_MODE_DEBOUNCE_TIME_MS))
        { 

            DEBUG("LONG PRESS START\n");
            ztimer_set(ZTIMER_MSEC, &long_press_timer, !buzzer_paired ? PAIRING_MODE_PRESS_DELAY_MS : RE_PAIRING_MODE_PRESS_DELAY_MS);
            mutex_lock(&led1_mutex);
            switch_activated = true;
            mutex_unlock(&led1_mutex);
            btn_long_press_debounce_ts = cur_ts;
        }
        /* if HAS been pressed but released now and outside of the debounce window */
        else if ((switch_activated && state == BUZZER_PASSIV_STATE) && (cur_ts - btn_long_press_debounce_ts >= PAIRING_MODE_DEBOUNCE_TIME_MS))
        {
            DEBUG("LONG PRESS END\n");
            mutex_lock(&led1_mutex);
            switch_activated = false;
            mutex_unlock(&led1_mutex);
            ztimer_remove(ZTIMER_MSEC, &long_press_timer);
            btn_long_press_debounce_ts = cur_ts;
        }
    }
    /* if buzzer is paired and not locked and outside of the debounce window */
    if (buzzer_paired && !buzzer_locked && (cur_ts - btn_debounce_ts >= DEBOUNCE_DELAY_MS))
    {
        // mutex_lock(&led1_mutex);

        DEBUG("BUZZER %d!\n", state);
       
        
        msg_send(&msg, *main_thread_pid);

        // mutex_unlock(&led1_mutex);
        btn_debounce_ts = cur_ts;
    }
}

void init_buzzer_periph(kernel_pid_t *main_thread_pid)
{

    gpio_init(led1, GPIO_OUT);
    gpio_init_int(btn, GPIO_IN_PU, GPIO_BOTH, btn_callback, (void *)main_thread_pid);

    long_press_timer.callback = long_press_callback;
    long_press_timer.arg = (void *)main_thread_pid;
}

void get_iso8601_time(char *buffer, size_t buffer_size)
{

    struct tm * tm_info;

    // uint64_t microseconds = sntp_get_unix_usec();
    

    // /* --- just for testing  --- */
    uint64_t microseconds = 1719846389000000;
    microseconds += xtimer_now_usec64();
    // /* --- just for testing  --- */

    time_t seconds = microseconds / 1000000;
    long remaining_microseconds = microseconds % 1000000;

    // Convert seconds since epoch to tm structure
    // gmtime_r(&seconds, tm_info);
    tm_info = gmtime(&seconds);
    // tm_info->tm_hour += TIME_ZONE_OFFSET_HOUR;
    tm_fill_derived_values(tm_info);

    (void) seconds;
    (void) remaining_microseconds;
    (void) buffer_size;
    (void) buffer;



    // DEBUG("TM: %d\n");   // tm_is_valid_date(tm_info.tm_year,tm_info.tm_mon, tm_info.tm_mday) 
       
    

    if(strftime(buffer, buffer_size, "%Y-%m-%dT%H:%M:%S", tm_info) <= 0){
        perror("Coulnd't write to time buffer!");
    }
    snprintf(buffer + strlen(buffer), buffer_size - strlen(buffer), ".%06ld", remaining_microseconds);
}

/**
 * @brief Handler for the '/buzzer/reset' resource. If called, the variable "buzzer_locked" will be set to false.
 *
 * @param pdu
 * @param buf
 * @param len
 * @param ctx
 * @return ssize_t
 */
ssize_t _buzzer_reset_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{
    (void)pdu;
    (void)buf;
    (void)len;

    kernel_pid_t *main_thread_pid = (kernel_pid_t *)ctx->resource->context;
    msg_t msg;

    if (buzzer_paired && buzzer_locked){
        DEBUG("RESET received!\n");
        msg.content.value = EVENT_ENABLE_NORMAL;

        msg_send(&msg, *main_thread_pid);
    }

    return coap_reply_simple(pdu, COAP_CODE_CHANGED, buf, len, 0, NULL, 0);
}

void init_buzzer_resources(kernel_pid_t *main_thread_pid)
{
    // snprintf(&_resource_uris[0][0], CONFIG_URI_MAX,"%s/reset",BUZZER_URI_BASE)


    _buzzer_resources[0].path = BUZZER_RESET_URI;
    _buzzer_resources[0].methods = COAP_PUT;
    _buzzer_resources[0].handler = _buzzer_reset_handler;
    _buzzer_resources[0].context = (void *)main_thread_pid;
    buzzer_resource_count++;


    _buzzer_resources[0].path = BUZZER_RESET_URI;
    _buzzer_resources[0].methods = COAP_PUT;
    _buzzer_resources[0].handler = _buzzer_reset_handler;
    _buzzer_resources[0].context = (void *)main_thread_pid;
    buzzer_resource_count++;

    _buzzer_listener.resources = &_buzzer_resources[0];
    _buzzer_listener.resources_len = (size_t)buzzer_resource_count;
    _buzzer_listener.tl_type = GCOAP_SOCKET_TYPE_UNDEF;
    _buzzer_listener.link_encoder = gcoap_encode_link;
    _buzzer_listener.next = NULL;
    _buzzer_listener.request_matcher = NULL;

    gcoap_register_listener(&_buzzer_listener);
}

int init_buzzer(kernel_pid_t *main_thread_pid)
{

    char *server_ip = "[2001:67c:254:b0b2:affe:4000:0:1]:123";
    // char server_buf[128];
    sock_udp_ep_t server;

    sock_udp_name2ep(&server, server_ip);
    int attempts = 0;
    int sntp_conn = -1;
    while ((attempts < MAX_SNTP_ATTEMPS) && (sntp_conn != 0))
    {
        sntp_conn = sntp_sync(&server, 3000 * 1000);
        attempts++;
    }
    if (attempts == MAX_SNTP_ATTEMPS)
    {
        DEBUG("[Error] Couldn't sync with sntp server: %d!", sntp_conn);
        return -1;
    }
    DEBUG("[INFO] SNTP Sync: %d, %d\n", sntp_conn, attempts);

    if (!init_done)
    {
        init_buzzer_resources(main_thread_pid);
        init_buzzer_periph(main_thread_pid);
        init_done = true;
    }

    return 0;
}

int get_time(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    char time_str[30];

    get_iso8601_time(time_str, sizeof(time_str));
    DEBUG("ISO Time: %s\n",time_str);

    return 0;
}