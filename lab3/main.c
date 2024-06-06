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

#include "ztimer.h"

#ifdef MODULE_NETIF
#include "net/gnrc/pktdump.h"
#include "net/gnrc.h"
#endif

#define MAX_RESOURCES 20
#define CONFIG_URI_MAX 128
#define MAIN_QUEUE_SIZE (4)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

#define URI_BASE "/b/"

// --

static ssize_t _riot_board_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

// coap_resource_t *_resources;
coap_resource_t _resources[MAX_RESOURCES];
char _resource_uris[MAX_RESOURCES][CONFIG_URI_MAX];
gcoap_listener_t _listener;


/*
 *   Handler for coap requests to read sensors or drive actuators via SAUL.
 */
static ssize_t _riot_board_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{

    const int dev_pos = (int) ctx->resource->context;
    
    saul_reg_t *dev = saul_reg_find_nth(dev_pos);
    int dev_class = dev->driver->type;

    /* read coap method type in packet */
    unsigned method_flag = coap_method2flag(coap_get_code_detail(pdu));
    // method_flag = 4; // just for testing

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
        if( dev_class == SAUL_ACT_LED_RGB ||
            dev_class == SAUL_SENSE_ACCEL || 
            dev_class == SAUL_SENSE_COLOR || 
            dev_class == SAUL_SENSE_MAG   || 
        )

        fmt_u16_dec(value, data.val[0]);

        /*get unit*/
        phydat_unit_write(unit, sizeof(unit), data.unit);

        /*get scale prefix*/
        char prefix = phydat_prefix_from_scale(data.scale);
        /* build response string
            => value + empty_space + prefix_char/e-xxx + unit
        */
        char response[10+5+1+5+1];
        int resp_str_len = 0;
        if(prefix != '\0' && data.scale== 0){
            resp_str_len = sprintf(response, "%s %c%s", value, prefix, unit);
        }else if (prefix == '\0' && data.scale != 0){

            resp_str_len = sprintf(response, "%se%d %s", value, data.scale, unit);
        }
        else{
            resp_str_len = sprintf(response, "%s %s", value, unit);
            
        }
        printf("%d",resp_str_len);

        memcpy(pdu->payload, response, resp_str_len);
        return resp_len + strlen(response);
    }
    else if (method_flag == COAP_PUT)
    {
        if (pdu->payload_len != 0)
        {   
            char payload[6] = {'\0'}; // max size 5 => "1,1,1"
            memcpy(payload, (char *)pdu->payload, pdu->payload_len);
            short int states[3] = {0};

            if (pdu->payload_len == 1){
                int tmp = atoi(payload);                
                states[0] = tmp;
            }else{
                char* pld = (char*) pdu->payload;
                (void) pld;
            }

            phydat_t data = {{*states}, UNIT_NONE, 0};


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
int init_board_periph_resources(void)
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

    if (dev_count > 0 && dev_count <= MAX_RESOURCES)
    {
        saul_reg_t *device = saul_reg;
        
        int i = 0;
        while (device)
        {
            /*
               uri: URI_BASE + device-name
            */

            int uri_len = snprintf(&_resource_uris[i][0], CONFIG_URI_MAX, "%s%s", URI_BASE, device->name);
            if(uri_len < 0 || uri_len > CONFIG_URI_MAX){
                printf("URI too long for: %s",device->name);
            }

            uint8_t class = device->driver->type >> 7; // get MSB
            
            /*
               class starts with 0b01xxxxxx => actuator
               class starts with 0b10xxxxxx => sensor
            */
            coap_method_flags_t methode = class == 0b0 ? COAP_GET | COAP_PUT : COAP_GET;


            
            _resources[i].path = &_resource_uris[i][0];
            _resources[i].methods = methode;
            _resources[i].handler = _riot_board_handler;
            _resources[i].context = (void *) i;

            device = device->next;
            i++;
        }
    }else if (dev_count <= MAX_RESOURCES){
        perror("Cant register all devices!");
        return 0;
    }

    return dev_count;

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



int main(void)
{
    int size = init_board_periph_resources();
    // int size = (sizeof((*_resources)) / sizeof((_resources)[0]));


    _listener.resources = &_resources[0];
    _listener.resources_len = size;
    _listener.tl_type = GCOAP_SOCKET_TYPE_UNDEF;
    _listener.link_encoder = gcoap_encode_link;
    _listener.next = NULL;
    _listener.request_matcher = NULL;



    

    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    server_init(_listener);
    (void)puts("CoAP Board Handler!");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
