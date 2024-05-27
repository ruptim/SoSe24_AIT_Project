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

static ssize_t _riot_board_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

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

/*
 *   Handler for coap requests to read sensors or drive actuators via SAUL.
 */
static ssize_t _riot_board_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{

    // size_t response_len = eval_device(ctx, pdu);
    char *dev_name = (char *)ctx->resource->context;
    saul_reg_t *dev = saul_reg_find_name(dev_name);

    /* read coap method type in packet */
    unsigned method_flag = coap_method2flag(coap_get_code_detail(pdu));
    method_flag = ctx->resource->methods; // just for testing

    if (method_flag == COAP_GET)
    {
        gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
        coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
        size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

        phydat_t data;
        char unit[10] = "";
        char value[5] = ""; // Max length of 2^16 is 5 (65536)

        /* get data */
        saul_reg_read(dev, &data);

        /*get value*/
        fmt_u16_dec(value, data.val[0]);

        /*get unit*/
        phydat_unit_write(unit, sizeof(unit), data.unit);

        /*get scale prefix*/
        char prefix = phydat_prefix_from_scale(data.scale);

        /* build response string
            => value + empty_space + prefix_char + unit
        */
        char response[strlen(value) + 1 + 1 + strlen(unit)];
        sprintf(response, "%s %c%s", value, prefix, unit);

        memcpy(pdu->payload, response, strlen(response));
        return resp_len + strlen(response);
    }
    else if (method_flag == COAP_PUT)
    {
        if (pdu->payload_len == 1)
        {
            uint8_t state = (uint8_t)*pdu->payload;
            phydat_t data = {{state, 0, 0}, UNIT_NONE, 0};
            saul_reg_write(dev, &data);

            return gcoap_response(pdu, buf, len, COAP_CODE_CHANGED);
        }
        else
        {
            return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
        }
    }

    return 0;
}

/*
 * Initializes coap resources for every saul device found for the current hardware.
 * The URI of a resource is created from the name of the corresponding device and the prefix URI_BASE.
 */
coap_resource_t *init_board_periph_resources(void)
{
    // printf("Seaching devices!");

    /* count registered saul devices */
    int dev_count = 0;
    if (saul_reg != NULL)
    {
        saul_reg_t *tmp = saul_reg;

        while (tmp)
        {
            printf("[Init] Found device: \"%s\"\n", tmp->name);
            tmp = tmp->next;
            dev_count++;
        }
    }

    /* create resource array and fill it with a resource per registered device */
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

/*
 *    function to test coap handler without coap
 */
static int hello_world(int argc, char **argv)
{
    /* Suppress compiler errors */
    (void)argc;
    (void)argv;

    int dev_id = 0;

    phydat_t data;
    saul_reg_t *dev = saul_reg_find_nth(dev_id);
    saul_reg_read(dev, &data);

    const coap_resource_t rs = _resources[dev_id];
    coap_request_ctx_t context = {&rs, NULL, 0};
    printf("Dev: %s, Class: %d, Method: %d,\n", (char *)rs.context, dev->driver->type, rs.methods);

    coap_pkt_t pdu;
    pdu.payload = (uint8_t *)(rs.methods == 4 ? !(bool)data.val[0] : 1); // if PUT, toggle value
    pdu.payload_len = 1;
    uint8_t buf[CONFIG_GCOAP_PDU_BUF_SIZE];

    _riot_board_handler(&pdu, &buf[0], CONFIG_GCOAP_PDU_BUF_SIZE, &context);
    printf("Payload: %s\n", (char *)pdu.payload);
    return 0;
}




int main(void)
{
    _resources = init_board_periph_resources();
    int size = (sizeof((*_resources)) / sizeof((_resources)[0]));

    gcoap_listener_t _listener = {
        &_resources[0],
        size,
        GCOAP_SOCKET_TYPE_UNDEF,
        gcoap_encode_link,
        NULL,
        NULL};

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
