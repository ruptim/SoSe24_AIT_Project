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

#define URI_BASE "/board/"

// --

// static ssize_t _encode_link(const coap_resource_t *resource, char *buf,
//                             size_t maxlen, coap_link_encoder_ctx_t *context);
static ssize_t _riot_board_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);
// static ssize_t _pressure_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

static int _request_matcher(gcoap_listener_t *listener, const coap_resource_t **resource, coap_pkt_t *pdu);

coap_resource_t *_resources;

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

// custom response matcher: nötig??
static int _request_matcher(gcoap_listener_t *listener,
                            const coap_resource_t **resource,
                            coap_pkt_t *pdu)
{
    uint8_t uri[CONFIG_NANOCOAP_URI_MAX];
    int ret = GCOAP_RESOURCE_NO_PATH;

    if (coap_get_uri_path(pdu, uri) <= 0)
    {
        /* The Uri-Path options are longer than
         * CONFIG_NANOCOAP_URI_MAX, and thus do not match anything
         * that could be found by this handler. */
        return GCOAP_RESOURCE_NO_PATH;
    }

    coap_method_flags_t method_flag = coap_method2flag(
        coap_get_code_detail(pdu));

    for (size_t i = 0; i < listener->resources_len; i++)
    {
        *resource = &listener->resources[i];

        int res = coap_match_path(*resource, uri);

        /* URI mismatch */
        if (res != 0)
        {
            continue;
        }

        /* potential match, check for method */
        if (!((*resource)->methods & method_flag))
        {
            /* record wrong method error for next iteration, in case
             * another resource with the same URI and correct method
             * exists */
            ret = GCOAP_RESOURCE_WRONG_METHOD;
            continue;
        }
        else
        {

            return GCOAP_RESOURCE_FOUND;
        }
    }

    return ret;
}

// char* format_value(phydat_t data){

//     return "";
// }

/*

*/
size_t eval_device(coap_request_ctx_t *ctx, coap_pkt_t *pdu)
{
    char *dev_name = (char *)ctx->resource->context;
    saul_reg_t *dev = saul_reg_find_name(dev_name);


    /* read coap method type in packet */
    unsigned method_flag = coap_method2flag(coap_get_code_detail(pdu));
    method_flag = ctx->resource->methods;
    // method_flag++;

    if (method_flag == COAP_GET)
    {
        phydat_t data;
        char unit[10] = "";

        saul_reg_read(dev, &data);
        phydat_unit_write(unit, sizeof(unit), data.unit);

        char prefix = phydat_prefix_from_scale(data.scale);
        printf("DATA READ: %d, %d, %c, %s\n", data.val[0], data.scale, prefix, unit);
        // uint16_t value = data.val[0];
        //TODO: convert value to correct output and fill reponse!!

        
                
        char response[20]; 
        memcpy(pdu->payload, response, strlen(response));
        return strlen(response);
    }
    else if (method_flag == COAP_PUT)
    {
        char *response;
        // TODO: check values
        int state = (int) *pdu->payload;
        phydat_t data = {{state, 0, 0}, UNIT_NONE, 0};
        saul_reg_write(dev, &data);
        response = "";
        memcpy(pdu->payload, response, strlen(response));
        return strlen(response);
    }
    char *response;
    response = "Request was not GET or PUT!";
    memcpy(pdu->payload, response, strlen(response));
    return strlen(response);
}

static ssize_t _riot_board_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{
    (void)ctx;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

    uint8_t uri[CONFIG_NANOCOAP_URI_MAX];

    if (coap_get_uri_path(pdu, uri) <= 0)
    {
        /* The Uri-Path options are longer than
         * CONFIG_NANOCOAP_URI_MAX, and thus do not match anything
         * that could be found by this handler. */
        perror("URI TO BIG!");
    }

    size_t response_len = eval_device(ctx, pdu);

    return resp_len + response_len;
}

// static ssize_t _pressure_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx) {
//     (void)ctx;
//     gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
//     coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
//     size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

//     phydat_t data;
//     saul_reg_read(saul_reg_find_nth(4), &data);
//     char str[20];

//     sprintf(str, "Val: %hi", *data.val); // todo: correct formatting?
//     /* Output is:
//         Showing hex dump of text/plain;charset=utf-8 payload: Message can not be parsed as UTF-8
//         00000000  56 61 6c 3a 20 30 00 1f  68 cf ff 1f 90 cf ff 1f  |Val: 0..h.......|
//         00000010  00 00 00 00                                       |....|
//         00000014
//      */

//     /* write the RIOT board name in the response buffer */
//     if (pdu->payload_len >= sizeof(str)) {
// //        memcpy(pdu->payload, RIOT_BOARD, strlen(RIOT_BOARD));
//         memcpy(pdu->payload, str, sizeof(str));
//         return resp_len + sizeof(str);
//     }
//     else {
//         puts("gcoap_cli: msg buffer too small");
//         return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
//     }
// }

void server_init(gcoap_listener_t listener)
{
#if IS_USED(MODULE_GCOAP_DTLS)
    int res = credman_add(&credential);
    if (res < 0 && res != CREDMAN_EXIST)
    {
        /* ignore duplicate credentials */
        printf("gcoap: cannot add credential to system: %d\n", res);
        return;
    }
    sock_dtls_t *gcoap_sock_dtls = gcoap_get_sock_dtls();
    res = sock_dtls_add_credential(gcoap_sock_dtls, GCOAP_DTLS_CREDENTIAL_TAG);
    if (res < 0)
    {
        printf("gcoap: cannot add credential to DTLS sock: %d\n", res);
    }
#endif

    gcoap_register_listener(&listener);
}

coap_resource_t *init_board_periph_resources(void)
{
    printf("Seaching devices!");
    int dev_count = 0;
    if (saul_reg != NULL)
    {
        saul_reg_t *tmp = saul_reg;

        while (tmp)
        {
            printf("[Init] Found device: \"%s\" %d\n", tmp->name, tmp->driver->type);
            tmp = tmp->next;
            dev_count++;
        }
    }
    coap_resource_t *arrayPtr = malloc(dev_count * sizeof(coap_resource_t));
    if (dev_count > 0)
    {
        saul_reg_t *device = saul_reg;

        int i = 0;
        while (device)
        {
            /*
               uri: URI_BASE + device-name
            */
            int uriLen = strlen(URI_BASE) + strlen(device->name);
            char *uri = malloc(uriLen * sizeof(char));
            sprintf(uri, "%s%s", URI_BASE, device->name);

            uint8_t class = device->driver->type >> 7; // get MSB

            /*
               class starts with 0b01xxxxxx => actuator
               class starts with 0b10xxxxxx => sensor
            */
            coap_method_flags_t methode = class == 0b0 ? COAP_PUT : COAP_GET;

            coap_resource_t res = {uri, methode, _riot_board_handler, (void *)device->name};
            arrayPtr[i] = res;

            device = device->next;
            i++;
        }
    }

    return arrayPtr;
}

static int hello_world(int argc, char **argv)
{
    /* Suppress compiler errors */
    (void)argc;
    (void)argv;

    int dev_id = 3;

    phydat_t data;
    saul_reg_t *dev = saul_reg_find_nth(dev_id);
    saul_reg_read(dev, &data);
    // printf("%s: %d\n", dev->name, data.val[0]);
    // LED_TOGGLE(0);

    const coap_resource_t rs = _resources[dev_id];
    coap_request_ctx_t context = {&rs, NULL, 0};
    printf("Dev: %s, %d, %d\n", (char *)rs.context, rs.methods,dev->driver->type);

    
    char pl[20] = "";
    coap_pkt_t pdu;
    pdu.payload = (uint8_t*) pl;
    pdu.payload_len = 0;
    eval_device(&context,&pdu);
    printf("%s\n",(char*)pdu.payload);
    return 0;
}

// const shell_command_t shell_commands[] = {
//     {"hello", "hello", hello_world},
//     {NULL, NULL, NULL}};

int main(void)
{
    _resources = init_board_periph_resources();
    int size = (sizeof((*_resources)) / sizeof((_resources)[0]));

    gcoap_listener_t _listener = {
        &_resources[0],
        size,
        GCOAP_SOCKET_TYPE_UNDEF,
        // _encode_link,
        gcoap_encode_link,
        NULL,
        _request_matcher};

    shell_command_t shell_commands[] = {
        {"hello", "hello", hello_world},
        {NULL, NULL, NULL}};

    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    server_init(_listener);
    (void)puts("CoAP Board Handler!");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
