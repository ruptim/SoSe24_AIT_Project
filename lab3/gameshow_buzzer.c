
#include "gameshow_buzzer.h"

#include "coap_client.h"

#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include "timex.h"
#include "xtimer.h"
#include "net/sntp.h"

#include "periph/gpio.h"
#include "ztimer.h"
#include "mutex.h"

#define MAX_RESOURCES 10
#define CONFIG_URI_MAX 128
#define BUZZER_RESET_URI "/buzzer/reset"
#define DEBOUNCE_DELAY_MS 20

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

bool buzzer_locked;
mutex_t buzzer_mutex = MUTEX_INIT;
unsigned int btn = GPIO_PIN(PORT_A, 2);
unsigned int led1 = GPIO_PIN(PORT_A, 1);


void send_test_data(void)
{

    char time_str[27];

    const char uri_base[128] = "coap://[2001:67c:254:b0b2:affe:4000:0:1]:9993";
    char *path = "/b/register";

    char payload[128] = "buzzer1";
    send_data(uri_base, path, (void *)payload, strlen(payload), NULL, NULL);

    get_iso8601_time(time_str, sizeof(time_str));

    xtimer_msleep(1000);

    char *path2 = "/b/pressed";
    strcat(payload, ",");
    strcat(payload, time_str);
    printf("SENDING: %s\n", payload);
    send_data(uri_base, path2, (void *)payload, strlen(payload), NULL, NULL);
}

static void btn_callback(void *arg)
{
    (void)arg;
    unsigned int *led1 = (unsigned int *)arg;

    ztimer_acquire(ZTIMER_MSEC);
    ztimer_now_t cur_ts = ztimer_now(ZTIMER_MSEC);
    ztimer_release(ZTIMER_MSEC);

    
    if(cur_ts - btn_debounce_ts >= DEBOUNCE_DELAY_MS){

        if (!buzzer_locked && *led1 != GPIO_UNDEF)
        {
            mutex_lock(&led1_mutex);

            gpio_toggle(*led1);
            printf("BUZZER!\n");

            // buzzer_locked = true;


            // send_test_data();

            mutex_unlock(&led1_mutex);

        }
        btn_debounce_ts = cur_ts;
    }
}

void init_buzzer_periph(void)
{
    // unsigned int btn = BTN0_PIN;
    // unsigned int led1 = LED0_PIN;

    gpio_init(led1, GPIO_OUT);
    gpio_init_int(btn, GPIO_IN_PU, GPIO_FALLING, btn_callback, (void*) &led1);
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

int get_time(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    char* server_ip = "[2001:67c:254:b0b2:affe:4000:0:1]:123";
    // char server_buf[128];
    sock_udp_ep_t server;
    
    sock_udp_name2ep(&server, server_ip);
    printf("SNTP Sync: %d\n",sntp_sync(&server, 3000*1000));

    if (!init_done)
    {   
      

        init_buzzer_resources();
        init_buzzer_periph();
        init_done = true;
        printf("INIT DONE \n");

    }

    send_test_data();

    // printf("ISO Time: %s\n",time_str);

    return 0;
}