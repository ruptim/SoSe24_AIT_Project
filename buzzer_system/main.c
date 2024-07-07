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


#include "../lab3/coap_server.h"
#include "../lab3/coap_client.h"
#include "../lab3/rd_registration.h"

#include "gameshow_buzzer.h"


#define MAIN_QUEUE_SIZE (8)

static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];


static const shell_command_t shell_commands[] = {
    { "time", "get ISO 8601 time", get_time },
    { NULL, NULL, NULL }
};


int main(void)
{
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    
    init_buzzer();

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    

    return 0;
}
