/*
 * Copyright (C) 2008, 2009, 2010 Kaspar Schleiser <kaspar@schleiser.de>
 * Copyright (C) 2013 INRIA
 * Copyright (C) 2013 Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Default application that shows a lot of functionality of RIOT
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>

#include "shell.h"

#include "led.h"

#include "fmt.h"
#include "net/gcoap.h"
#include "net/utils.h"
#include "od.h"
#include "saul_reg.h"
#include "phydat.h"

#ifdef MODULE_NETIF
    #include "net/gnrc/pktdump.h"
    #include "net/gnrc.h"
#endif


#define MAIN_QUEUE_SIZE (4)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

/* internal value that can be read/written via CoAP */
static uint8_t internal_value = 0;


// --



// static ssize_t _encode_link(const coap_resource_t *resource, char *buf,
//                             size_t maxlen, coap_link_encoder_ctx_t *context);
static ssize_t _riot_board_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);
static ssize_t _pressure_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);
static ssize_t _read_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);
static ssize_t _echo_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);
static ssize_t _value_handler(coap_pkt_t* pkt, uint8_t *buf, size_t len, coap_request_ctx_t *context);
static ssize_t _sensors_handler(coap_pkt_t* pkt, uint8_t *buf, size_t len, coap_request_ctx_t *context);

static const coap_resource_t _resources[] = {
    { "/riot/board/humidity", COAP_GET, _riot_board_handler, NULL },
    {"/riot/board/pressure", COAP_GET, _pressure_handler, NULL},
    {"/riot/board/read", COAP_GET | COAP_MATCH_SUBTREE, _read_handler, NULL},
    {"/echo/", COAP_GET | COAP_MATCH_SUBTREE, _echo_handler, NULL},
    {"/riot/value", COAP_GET | COAP_PUT | COAP_POST | COAP_MATCH_SUBTREE, _value_handler, NULL},
    {"/riot/sensors", COAP_GET | COAP_PUT | COAP_POST | COAP_MATCH_SUBTREE, _sensors_handler, NULL}
};


static gcoap_listener_t _listener = {
    &_resources[0],
    ARRAY_SIZE(_resources),
    GCOAP_SOCKET_TYPE_UNDEF,
    // _encode_link,
    gcoap_encode_link,
    NULL,
    NULL
};


// /* Adds link format params to resource list */
// static ssize_t _encode_link(const coap_resource_t *resource, char *buf,
//                             size_t maxlen, coap_link_encoder_ctx_t *context) {
//     ssize_t res = gcoap_encode_link(resource, buf, maxlen, context);
//     if (res > 0) {
//         if (_link_params[context->link_pos]
//                 && (strlen(_link_params[context->link_pos]) < (maxlen - res))) {
//             if (buf) {
//                 memcpy(buf+res, _link_params[context->link_pos],
//                        strlen(_link_params[context->link_pos]));
//             }
//             return res + strlen(_link_params[context->link_pos]);
//         }
//     }

//     return res;
// }


static ssize_t _riot_board_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{
    (void)ctx;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

    /* write the RIOT board name in the response buffer */
    if (pdu->payload_len >= strlen(RIOT_BOARD)) {
        memcpy(pdu->payload, RIOT_BOARD, strlen(RIOT_BOARD));
        return resp_len + strlen(RIOT_BOARD);
    }
    else {
        puts("gcoap_cli: msg buffer too small");
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }
}

static ssize_t _pressure_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx) {
    (void)ctx;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

    phydat_t data;
    saul_reg_read(saul_reg_find_nth(4), &data);
    char str[20];

    sprintf(str, "Val: %hi", *data.val); // todo: correct formatting?
    /* Output is:
        Showing hex dump of text/plain;charset=utf-8 payload: Message can not be parsed as UTF-8
        00000000  56 61 6c 3a 20 30 00 1f  68 cf ff 1f 90 cf ff 1f  |Val: 0..h.......|
        00000010  00 00 00 00                                       |....|
        00000014
     */

    /* write the RIOT board name in the response buffer */
    if (pdu->payload_len >= sizeof(str)) {
//        memcpy(pdu->payload, RIOT_BOARD, strlen(RIOT_BOARD));
        memcpy(pdu->payload, str, sizeof(str));
        return resp_len + sizeof(str);
    }
    else {
        puts("gcoap_cli: msg buffer too small");
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }
}

static ssize_t _echo_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{
    (void)ctx;
    char uri[CONFIG_NANOCOAP_URI_MAX];

    if (coap_get_uri_path(pdu, (uint8_t *)uri) <= 0) {
        return coap_reply_simple(pdu, COAP_CODE_INTERNAL_SERVER_ERROR, buf,
                                 len, COAP_FORMAT_TEXT, NULL, 0);
    }
    char *sub_uri = uri + strlen("/echo/");
    size_t sub_uri_len = strlen(sub_uri);
    return coap_reply_simple(pdu, COAP_CODE_CONTENT, buf, len, COAP_FORMAT_TEXT,
                             (uint8_t *)sub_uri, sub_uri_len);
}

static ssize_t _value_handler(coap_pkt_t *pkt, uint8_t *buf, size_t len, coap_request_ctx_t *context)
{
    (void) context;

    ssize_t p = 0;
    char rsp[16];
    unsigned code = COAP_CODE_EMPTY;

    /* read coap method type in packet */
    unsigned method_flag = coap_method2flag(coap_get_code_detail(pkt));

    switch(method_flag) {
        case COAP_GET:
            /* write the response buffer with the internal value */
            p += fmt_u32_dec(rsp, internal_value);
            code = COAP_CODE_205;
            break;
        case COAP_PUT:
        case COAP_POST:
            if (pkt->payload_len < 16) {
                /* convert the payload to an integer and update the internal value */
                char payload[16] = { 0 };
                memcpy(payload, (char*)pkt->payload, pkt->payload_len);
                internal_value = strtol(payload, NULL, 10);
                code = COAP_CODE_CHANGED;
            }
            else {
                code = COAP_CODE_REQUEST_ENTITY_TOO_LARGE;
            }
    }

    return coap_reply_simple(pkt, code, buf, len,
                             COAP_FORMAT_TEXT, (uint8_t*)rsp, p);
}

static ssize_t _sensors_handler(coap_pkt_t *pkt, uint8_t *buf, size_t len, coap_request_ctx_t *context)
{
    (void) context;
    char uri[CONFIG_NANOCOAP_URI_MAX];

    size_t resp = 0;
    char rsp[16];
    unsigned code = COAP_CODE_EMPTY;

    if (coap_get_uri_path(pdu, (uint8_t *)uri) <= 0) {
        return coap_reply_simple(pdu, COAP_CODE_INTERNAL_SERVER_ERROR, buf,
                                 len, COAP_FORMAT_TEXT, NULL, 0);
    }
    char *sub_uri = uri + strlen("/riot/sensors/");
    size_t sub_uri_len = strlen(sub_uri);
    char *sensor_id = sub_uri[0];
    char str[80];



    /* read coap method type in packet */
    unsigned method_flag = coap_method2flag(coap_get_code_detail(pkt));

    switch(method_flag) {
        case COAP_GET:
            strcpy(str, "GET ");
            strcat(str, sensor_id);
            break;
        case COAP_PUT:
        case COAP_POST:
            strcpy(str, "POST ");
            strcat(str, sensor_id);
            break;
    }

    size_t str_len = strlen(str);

    return coap_reply_simple(pdu, COAP_CODE_CONTENT, buf, len, COAP_FORMAT_TEXT,
                             (uint8_t *)str, str_len);
}

static ssize_t _read_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx) {
    (void)ctx;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

    uint8_t uri[CONFIG_NANOCOAP_URI_MAX];
    coap_get_uri_path(pdu, &uri[0]);
    char* uri_char = (char *) uri;

    // TODO use chars after uri to filter integer parameters
    size_t length = strlen(uri_char);
    char last_chars[20] = ""; // To store the last 1 characters plus null terminator
    if (length >= 1) {
        strncpy(last_chars, uri_char, sizeof(char)*19);
//        last_chars[1] = '\0'; // Null-terminate the string
    }

    // TODO update
    if (pdu->payload_len >= strlen(last_chars)) {
        memcpy(pdu->payload, last_chars, strlen(last_chars));
        return resp_len + strlen(last_chars);
    }
    else {
        puts("gcoap_cli: msg buffer too small");
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }
}


void server_init(void)
{
#if IS_USED(MODULE_GCOAP_DTLS)
    int res = credman_add(&credential);
    if (res < 0 && res != CREDMAN_EXIST) {
        /* ignore duplicate credentials */
        printf("gcoap: cannot add credential to system: %d\n", res);
        return;
    }
    sock_dtls_t *gcoap_sock_dtls = gcoap_get_sock_dtls();
    res = sock_dtls_add_credential(gcoap_sock_dtls, GCOAP_DTLS_CREDENTIAL_TAG);
    if (res < 0) {
        printf("gcoap: cannot add credential to DTLS sock: %d\n", res);
    }
#endif

    gcoap_register_listener(&_listener);
}







static int hello_world(int argc, char **argv) {
    /* Suppress compiler errors */
    (void)argc;
    (void)argv;
    printf("Toggle LED 0!\n");
    LED_TOGGLE(0);
    return 0;
}

const shell_command_t shell_commands[] = {
    {"hello", "prints hello world", hello_world},
    { NULL, NULL, NULL }
};





int main(void)
{   
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    server_init();
    (void) puts("CoAP Board Handler!");
	
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
