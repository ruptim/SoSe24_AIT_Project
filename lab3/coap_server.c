/*
 * Copyright (c) 2015-2017 Ken Bannister. All rights reserved.
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
 * @brief       gcoap CLI support
 *
 * @author      Ken Bannister <kb2ma@runbox.com>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fmt.h"
#include "net/gcoap.h"
#include "net/utils.h"
#include "od.h"
#include "saul_reg.h"
#include "phydat.h"

#include "coap_server.h"

#define ENABLE_DEBUG 1
#include "debug.h"

coap_resource_t _resources[MAX_RESOURCES];
char _resource_uris[MAX_RESOURCES][CONFIG_URI_MAX];
gcoap_listener_t _listener;

/* Adds link format params to resource list */
ssize_t _encode_link(const coap_resource_t *resource, char *buf,
                     size_t maxlen, coap_link_encoder_ctx_t *context)
{
    ssize_t res = gcoap_encode_link(resource, buf, maxlen, context);

    char link[MAX_LINK_LENGTH] = "";
    if (resource->methods == COAP_GET)
    {
        snprintf(link,MAX_LINK_LENGTH,";G");
    }
    else if (resource->methods == (COAP_PUT | COAP_POST | COAP_GET))
    {
        snprintf(link,MAX_LINK_LENGTH,";G|P");
    }

    if (res > 0)
    {
        if ((strlen(link) < (maxlen - res)))
        {
            if (buf)
            {
                memcpy(buf + res, link,
                       strlen(link));
            }
            return res + strlen(link);
        }
    }

    return res;
}

/**
 * Extract rgb values froma payload string to an array.
 * String format: "R,G,B".
 * Length varies from "0,0,0" up to "255,255,255".
 */
int get_rgb_values(short int *states, char *payload)
{
    int r = atoi(payload);
    int r_size = snprintf(NULL, 0, "%d", r) + 1;
    int g = atoi(&payload[r_size]);
    int g_size = snprintf(NULL, 0, "%d", g) + 1;
    int b = atoi(&payload[r_size + g_size]);

    if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255)
    {
        states[0] = r;
        states[1] = g;
        states[2] = b;

        return 0;
    }
    else
    {
        return -1;
    }
}

ssize_t _riot_board_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{
    bool observer_req = false;
    if (coap_has_observe(pdu))
    {
        uint32_t obs = coap_get_observe(pdu);
        observer_req = obs == 0;
    }

    const int dev_pos = (int)ctx->resource->context;

    saul_reg_t *dev = saul_reg_find_nth(dev_pos);
    int dev_class = dev->driver->type;

    /* read coap method type in packet */
    unsigned method_flag = coap_method2flag(coap_get_code_detail(pdu));
    // method_flag = 4; // just for testing

    if (method_flag == COAP_GET)
    {

        if (observer_req)
        {
            /* important: dev position is equal to resource index
               ONLY if there are no other resources than saul devices */
            switch (gcoap_obs_init(pdu, &buf[0], CONFIG_GCOAP_PDU_BUF_SIZE, &_resources[dev_pos]))
            {
            case GCOAP_OBS_INIT_OK:
                DEBUG("gcoap_cli: creating /cli/stats notification\n");

                //     break;
                // case GCOAP_OBS_INIT_UNUSED:
                //     DEBUG("gcoap_cli: no observer for /cli/stats\n");
                //     break;
                // case GCOAP_OBS_INIT_ERR:
                //     DEBUG("gcoap_cli: error initializing /cli/stats notification\n");
                //     break;
            }
        }
        else
        {
            gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
        }
        coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
        size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

        phydat_t data;
        char unit[10] = "";
        char value[3][5] = {"", "", ""}; // Max length of 2^16 is 5 (65536)
        int values_to_send = 1;
        /* get data */
        saul_reg_read(dev, &data);
        /*get value*/
        fmt_s16_dec(value[0], data.val[0]);

        if (dev_class == SAUL_ACT_LED_RGB ||
            dev_class == SAUL_SENSE_ACCEL ||
            dev_class == SAUL_SENSE_COLOR ||
            dev_class == SAUL_SENSE_MAG)
        {
            fmt_u16_dec(value[1], data.val[1]);
            fmt_u16_dec(value[2], data.val[2]);
            values_to_send = 3;
        }

        /*get unit*/
        phydat_unit_write(unit, sizeof(unit), data.unit);

        /*get scale prefix*/
        char prefix = phydat_prefix_from_scale(data.scale);
        /* build response string
            => value + empty_space + prefix_char/e-xxx + unit
        */

        char response[MAX_GET_PAYLOAD_LEN];
        char *response_ptr = (char *)response;
        int resp_str_len = snprintf(response_ptr, MAX_GET_PAYLOAD_LEN, "%s: ", dev->name);
        response_ptr += resp_str_len;

        for (int i = 0; i < values_to_send; i++)
        {
            int value_len = 0;
            if (prefix != '\0' && data.scale == 0)
            {
                value_len = snprintf(response_ptr, MAX_GET_PAYLOAD_LEN, "%s %c%s", value[0], prefix, unit);
            }
            else if (prefix == '\0' && data.scale != 0)
            { /* no prefix => scientific notation */

                value_len = snprintf(response_ptr, MAX_GET_PAYLOAD_LEN, "%se%d %s", value[0], data.scale, unit);
            }
            else
            { /* no prefix and no sclaing */
                value_len = snprintf(response_ptr, MAX_GET_PAYLOAD_LEN, "%s %s", value[0], unit);
            }
            resp_str_len += value_len;
            response_ptr += value_len;
            if (values_to_send == 3)
            {
                int value_len_2 = snprintf(response_ptr, MAX_GET_PAYLOAD_LEN, ", ");
                resp_str_len += value_len_2;
                response_ptr += value_len_2;
            }
        }

        memcpy(pdu->payload, response, resp_str_len);

        if (observer_req)
        {
            DEBUG("Sending notificatoin!");
            return gcoap_obs_send(&buf[0], resp_len + strlen(response), &_resources[dev_pos]);
        }
        DEBUG("Sending response!");
        return resp_len + strlen(response);
    }
    else if (method_flag == COAP_PUT || method_flag == COAP_POST)
    {
        if (pdu->payload_len != 0 && pdu->payload_len != MAX_PUT_PAYLOAD_LEN)
        {
            char payload[MAX_PUT_PAYLOAD_LEN] = {'\0'}; // max size 11 => "255,255,255"
            memcpy(payload, (char *)pdu->payload, pdu->payload_len);
            short int states[3] = {0};

            if (pdu->payload_len == 1)
            {
                int tmp = atoi(payload);
                states[0] = tmp;
            }
            else if (pdu->payload_len >= 5)
            {

                char *pld = (char *)pdu->payload;
                (void)pld;

                if (get_rgb_values(&states[0], pld) == -1)
                {
                    return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
                }

                //// -------------  sscanf results in panic! --------------
                // if(sscanf(paykd, "%d,%d,%d", &r, &g, &b) != 3){
                // if(sscanf(pld, "%hd,%hd,%hd", &states[0], &states[1], &states[3]) != 3){
                //     return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
                // }
            }
            else
            {
                return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
            }

            /* write data to register */
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
 * The URI of a resource is created from the name of the prefix URI_BASE and the corresponding device name and id.
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
               uri: URI_BASE + device-name + _id
            */

            // int uri_len = snprintf(&_resource_uris[i][0], CONFIG_URI_MAX, "%s%s_%d", URI_BASE, device->name, i);
            int uri_len = snprintf(&_resource_uris[i][0], CONFIG_URI_MAX, "%s%d", URI_BASE, i);
            if (uri_len < 0 || uri_len > CONFIG_URI_MAX)
            {
                printf("URI too long for: %s", device->name);
            }

            uint8_t class = device->driver->type >> 7; // get MSB

            /*
               class starts with 0b01xxxxxx => actuator
               class starts with 0b10xxxxxx => sensor
            */
            coap_method_flags_t methode = class == 0b0 ? COAP_PUT | COAP_POST | COAP_GET : COAP_GET;

            _resources[i].path = &_resource_uris[i][0];
            _resources[i].methods = methode;
            _resources[i].handler = _riot_board_handler;
            _resources[i].context = (void *)i;

            device = device->next;
            i++;
        }
    }
    else if (dev_count <= MAX_RESOURCES)
    {
        perror("Cant register all devices!");
        return 0;
    }

    return dev_count;
}

void notify_observers(void)
{
    // size_t len;
    uint8_t buf[CONFIG_GCOAP_PDU_BUF_SIZE];
    coap_pkt_t pdu;

    // TODO: check which resource is requested -------------

    /* send Observe notification for /cli/stats */
    switch (gcoap_obs_init(&pdu, &buf[0], CONFIG_GCOAP_PDU_BUF_SIZE,
                           &_resources[0]))
    {
    case GCOAP_OBS_INIT_OK:
        DEBUG("gcoap_cli: creating /cli/stats notification\n");
        // coap_opt_add_format(&pdu, COAP_FORMAT_TEXT);
        // len = coap_opt_finish(&pdu, COAP_OPT_FINISH_PAYLOAD);
        // len += fmt_u16_dec((char *)pdu.payload, req_count);
        // gcoap_obs_send(&buf[0], len, &_resources[0]);
        break;
    case GCOAP_OBS_INIT_UNUSED:
        DEBUG("gcoap_cli: no observer for /cli/stats\n");
        break;
    case GCOAP_OBS_INIT_ERR:
        DEBUG("gcoap_cli: error initializing /cli/stats notification\n");
        break;
    }
}

void server_init(gcoap_listener_t *listener)
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

    gcoap_register_listener(listener);
}
