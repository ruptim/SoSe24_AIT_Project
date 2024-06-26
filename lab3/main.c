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
#include <stdlib.h>
#include <string.h>

#include "shell.h"

#include "led.h"

#include "fmt.h"
#include "net/gcoap.h"
#include "net/utils.h"
#include "od.h"
#include "saul_reg.h"
#include "phydat.h"

#include "net/ipv6/addr.h"

#include "thread.h"


#ifdef MODULE_NETIF
#include "net/gnrc/pktdump.h"
#include "net/gnrc.h"
#endif


#include "coap_server.h"
#include "coap_client.h"
#include "rd_registration.h"

#define MAIN_QUEUE_SIZE (8)

static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

char rd_register_thread_stack[THREAD_STACKSIZE_MAIN];

static const shell_command_t shell_commands[] = {
    { "coap", "CoAP client", gcoap_cli_cmd },
    { NULL, NULL, NULL }
};


int main(void)
{
    int size = init_board_periph_resources();


    _listener.resources = &_resources[0];
    _listener.resources_len = (size_t) size;
    _listener.tl_type = GCOAP_SOCKET_TYPE_UNDEF;
    _listener.link_encoder = gcoap_encode_link;
    _listener.next = NULL;
    _listener.request_matcher = NULL;
    
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    server_init(&_listener);
    (void)puts("CoAP Board Handler!");

    /* register event callback with cord_ep_standalone */
    // cord_ep_standalone_reg_cb(_on_ep_event);
    // puts("Client information:");
    // printf("  ep: %s\n", cord_common_get_ep());
    // printf("  lt: %is\n", (int)CONFIG_CORD_LT);

    
    thread_create(rd_register_thread_stack, sizeof(rd_register_thread_stack),
            THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
            register_at_resource_directory, NULL, "rcv_thread");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    

    return 0;
}
