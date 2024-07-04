
#include "gameshow_buzzer.h"

#include "../lab3/coap_client.h"
#include "buzzer_communication.h"

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
#include "thread.h"

#define MAX_RESOURCES 10
#define CONFIG_URI_MAX 128
#define BUZZER_RESET_URI "/buzzer/reset"
#define DEBOUNCE_DELAY_MS 300
#define BUZZER_ACTIVE_STATE 0
#define BUZZER_PASSIV_STATE 1
#define BUZZER_PAIRING_MODE_BLINK_DELAY_MS 200
#define BUZZER_NOT_CONNECTED_BLINK_DELAY_MS 1000

#define MAX_SNTP_ATTEMPS 5
#define MAX_PAIRING_ATTEMPS 1
#define BUZZER_STARTUP_DELAY_MS 2000
#define PAIRING_MODE_PRESS_DELAY_MS 2000
#define PAIRING_MODE_DEBOUNCE_TIME_MS 10
#define TIME_ZONE_OFFSET_HOUR 2

// #define BUZZER_SWITCH_PIN
// #define BUZZER_LED_PIN PORT_E, 18

coap_resource_t _buzzer_resources[MAX_RESOURCES];
char _buzzer_resource_uris[MAX_RESOURCES][CONFIG_URI_MAX];
uint8_t buzzer_resource_count = 0;
gcoap_listener_t _buzzer_listener;

ztimer_now_t btn_debounce_ts;
ztimer_t timer;
mutex_t led1_mutex = MUTEX_INIT;

bool init_done = false;
bool buzzer_paired = false;
bool buzzer_not_connected_mode = true;
bool buzzer_pairing_mode = false;
bool buzzer_pairing_mode_timeout = false;
bool buzzer_locked = false;
bool switch_activated = false;

mutex_t buzzer_mutex = MUTEX_INIT;
unsigned int btn = GPIO_PIN(PORT_A, 2);
ztimer_t long_press_timer;
unsigned int led1 = GPIO_PIN(PORT_A, 1);

char buzzer_pairing_thread_stack[THREAD_STACKSIZE_DEFAULT];
char buzzer_blink_thread_stack[THREAD_STACKSIZE_DEFAULT];
char buzzer_pairing_blink_thread_stack[THREAD_STACKSIZE_DEFAULT];

const char uri_base[128] = "coap://[2001:67c:254:b0b2:affe:4000:0:1]:9993";

enum BLINK_MODES
{
    BLINK_NOT_CONN,
    BLINK_PAIR_MODE,
};

void send_test_data(void)
{

    char time_str[27];

    char *path = "/b/register";

    char payload[128] = "buzzer1";
    send_data(uri_base, path, (void *)payload, strlen(payload), NULL, NULL, true);

    get_iso8601_time(time_str, sizeof(time_str));

    xtimer_msleep(1000);

    char *path2 = "/b/pressed";
    strcat(payload, ",");
    strcat(payload, time_str);
    printf("SENDING: %s\n", payload);
    send_data(uri_base, path2, (void *)payload, strlen(payload), NULL, NULL, false);
}


void send_buzzer_pressed(void)
{
    char time_str[27];
    char payload[128] = "buzzer1";

    get_iso8601_time(time_str, sizeof(time_str));

    char *path = "/b/pressed";
    strcat(payload, ",");
    strcat(payload, time_str);
    printf("SENDING: %s\n", payload);
    send_data(uri_base, path, (void *)payload, strlen(payload), NULL, NULL, false);
}

void pair_resp_handler(const gcoap_request_memo_t *memo, coap_pkt_t *pdu,
                       const sock_udp_ep_t *remote)
{
    (void)pdu;
    (void)remote;
    /* response timeout or error in response */
    if (memo->state == GCOAP_MEMO_TIMEOUT || memo->state != GCOAP_MEMO_RESP)
    {
        buzzer_pairing_mode_timeout = true;
        printf("PAIR TIMEOUT\n");
        return;
    }
    printf("BUZZER PAIRED\n");


    //TODO: make shure that no buzzer_pressed is send when releasing the long press 
    // happens when pairing happens fast before the buzzer is released again!!
    // ---------------


    ztimer_acquire(ZTIMER_MSEC);
    btn_debounce_ts = ztimer_now(ZTIMER_MSEC);
    ztimer_release(ZTIMER_MSEC);

    buzzer_paired = true;
}

void send_pair_request(void)
{
    char *path = "/b/register";

    char payload[128] = "buzzer1";
    send_data(uri_base, path, (void *)payload, strlen(payload), pair_resp_handler, NULL, true);
}

void *pairing_blink_routine(void *args)
{
    (void)args;

    printf("START PAIRING BLINK\n");

    while (buzzer_pairing_mode)
    {
        gpio_toggle(led1);
        ztimer_sleep(ZTIMER_MSEC, BUZZER_PAIRING_MODE_BLINK_DELAY_MS);
    }
    printf("STOPPED PAIRING BLINK\n");

    return 0;
}

void *not_connected_blink_routine(void *args)
{
    (void)args;

    printf("START NOT CONNECTED BLINK\n");
    
    while (buzzer_not_connected_mode)
    {
        gpio_toggle(led1);
        ztimer_sleep(ZTIMER_MSEC, BUZZER_NOT_CONNECTED_BLINK_DELAY_MS);
    }
    printf("STOPPED NOT CONNECTED BLINK\n");

    return 0;
}


void start_not_conn_blink_thread(void){
    buzzer_not_connected_mode = true;
    thread_create(buzzer_blink_thread_stack, sizeof(buzzer_blink_thread_stack),
                THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
                not_connected_blink_routine, NULL, "not_conn_pairing_thread");
}


void *pairing_mode_routine(void *args)
{
    (void)args;

    buzzer_pairing_mode = true;

    thread_create(buzzer_pairing_blink_thread_stack, sizeof(buzzer_pairing_blink_thread_stack),
                  THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
                  pairing_blink_routine, NULL, "blink_thread");

    int attemps = MAX_PAIRING_ATTEMPS;
    while (!buzzer_paired && attemps != 0)
    {
        printf("PAIRING MODE!!\n");
        buzzer_pairing_mode_timeout = false;
        send_pair_request();
        while (!buzzer_paired && !buzzer_pairing_mode_timeout)
        {
            ztimer_sleep(ZTIMER_MSEC, 100);
        }
        attemps--;
    }
    buzzer_pairing_mode = false;

    if(!buzzer_paired){
        start_not_conn_blink_thread();
    }else{
        
    }


    return 0;
}

void long_press_callback(void *args)
{
    (void)args;

    mutex_lock(&led1_mutex);
    switch_activated = false;
    mutex_unlock(&led1_mutex);

    printf("LONG PRESS REGISTERED!\n");
    
    buzzer_not_connected_mode = false;

    thread_create(buzzer_pairing_thread_stack, sizeof(buzzer_pairing_thread_stack),
                  THREAD_PRIORITY_MAIN - 2, THREAD_CREATE_STACKTEST,
                  pairing_mode_routine, NULL, "pairing_thread");
}

static void btn_callback(void *args)
{
    (void)args;

    ztimer_acquire(ZTIMER_MSEC);
    ztimer_now_t cur_ts = ztimer_now(ZTIMER_MSEC);
    ztimer_release(ZTIMER_MSEC);

    int state = gpio_read(btn);

    /* if buzzer is not locked */
    if (!buzzer_paired && !buzzer_pairing_mode)
    {
        /* if HASN'T been pressed yet but pressed now and outside of the debounce window */
        if ((!switch_activated && state == BUZZER_ACTIVE_STATE) && (cur_ts - btn_debounce_ts >= PAIRING_MODE_DEBOUNCE_TIME_MS))
        {

            printf("LONG PRESS START\n");
            ztimer_set(ZTIMER_MSEC, &long_press_timer, PAIRING_MODE_PRESS_DELAY_MS);
            mutex_lock(&led1_mutex);
            switch_activated = true;
            mutex_unlock(&led1_mutex);
            switch_activated = true;
            btn_debounce_ts = cur_ts;
        }
        /* if HAS been pressed but released now and outside of the debounce window */
        else if ((switch_activated && state == BUZZER_PASSIV_STATE) && (cur_ts - btn_debounce_ts >= PAIRING_MODE_DEBOUNCE_TIME_MS))
        {
            printf("LONG PRESS END\n");
            mutex_lock(&led1_mutex);
            switch_activated = false;
            mutex_unlock(&led1_mutex);
            ztimer_remove(ZTIMER_MSEC, &long_press_timer);
        }
    }
    else if (buzzer_paired && !buzzer_locked && (cur_ts - btn_debounce_ts >= DEBOUNCE_DELAY_MS))
    {
        mutex_lock(&led1_mutex);

        gpio_toggle(led1);
        printf("BUZZER %d!\n", state);
        send_buzzer_pressed();

        // buzzer_locked = true;

        // send_test_data();

        mutex_unlock(&led1_mutex);
        btn_debounce_ts = cur_ts;
    }
}


void init_buzzer_periph(void)
{
    // unsigned int btn = BTN0_PIN;
    // unsigned int led1 = LED0_PIN;

    gpio_init(led1, GPIO_OUT);
    gpio_init_int(btn, GPIO_IN_PU, GPIO_BOTH, btn_callback, NULL);

    long_press_timer.callback = long_press_callback;

    start_not_conn_blink_thread();

    
}

void get_iso8601_time(char *buffer, size_t buffer_size)
{

    struct tm tm_info;
    (void)tm_info;

    uint64_t microseconds = sntp_get_unix_usec();

    // /* --- just for testing  --- */
    // uint64_t microseconds = 1719846389000000;
    // microseconds += xtimer_now_usec64();
    // /* --- just for testing  --- */

    time_t seconds = microseconds / 1000000;
    long remaining_microseconds = microseconds % 1000000;

    // Convert seconds since epoch to tm structure
    gmtime_r(&seconds, &tm_info);
    tm_info.tm_hour += TIME_ZONE_OFFSET_HOUR;

    strftime(buffer, buffer_size, "%Y-%m-%dT%H:%M:%S", &tm_info);
    snprintf(buffer + strlen(buffer), buffer_size - strlen(buffer), ".%06ldZ", remaining_microseconds);
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
    (void)ctx;

    mutex_lock(&buzzer_mutex);
    buzzer_locked = false;
    mutex_unlock(&buzzer_mutex);

    return coap_reply_simple(pdu, COAP_CODE_CHANGED, buf, len, 0, NULL, 0);
}

void init_buzzer_resources(void)
{
    // snprintf(&_resource_uris[0][0], CONFIG_URI_MAX,"%s/reset",BUZZER_URI_BASE)

    _buzzer_resources[0].path = BUZZER_RESET_URI;
    _buzzer_resources[0].methods = COAP_PUT;
    _buzzer_resources[0].handler = _buzzer_reset_handler;
    buzzer_resource_count++;

    _buzzer_listener.resources = &_buzzer_resources[0];
    _buzzer_listener.resources_len = (size_t)buzzer_resource_count;
    _buzzer_listener.tl_type = GCOAP_SOCKET_TYPE_UNDEF;
    _buzzer_listener.link_encoder = gcoap_encode_link;
    _buzzer_listener.next = NULL;
    _buzzer_listener.request_matcher = NULL;

    gcoap_register_listener(&_buzzer_listener);
}

int init_buzzer(void)
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
        printf("[Error] Couldn't sync with sntp server: %d!", sntp_conn);
        return -1;
    }
    printf("SNTP Sync: %d, %d\n", sntp_conn, attempts);

    if (!init_done)
    {
        init_buzzer_resources();
        init_buzzer_periph();
        init_done = true;
        printf("INIT DONE \n");
    }

    return 0;
}

int get_time(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    buzzer_paired = true;

    send_test_data();

    // printf("ISO Time: %s\n",time_str);

    return 0;
}