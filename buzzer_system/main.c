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
#include "msg.h"

#ifdef MODULE_NETIF
#include "net/gnrc/pktdump.h"
#include "net/gnrc.h"
#endif


#include "../lab3/coap_server.h"
#include "../lab3/coap_client.h"
#include "../lab3/rd_registration.h"

#include "gameshow_buzzer.h"
#include "defines.h"


#define MAIN_QUEUE_SIZE (8)
#define RCV_QUEUE_SIZE  (8)

static kernel_pid_t rcv_pid;
static char main_routine_stack[THREAD_STACKSIZE_DEFAULT + THREAD_EXTRA_STACKSIZE_PRINTF];
static msg_t rcv_queue[RCV_QUEUE_SIZE];

static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];


static const shell_command_t shell_commands[] = {
    { "time", "get ISO 8601 time", get_time },
    { NULL, NULL, NULL }
};


//TODO: create main thread which is in charge of the modes
//      Mode 0 NOT_CONN: start blink thread
//             => When long_press recognized -> MSG to main thread (mode 1)
//      Mode 1 PAIRING: start pairing and pairing blink thread
//             => if failed -> MSG to main thread (Mode 0)
//             => if succeded -> MSG to main thread (Mode 3)
//      Mode 2 NORMAL: normal buzzer behaviour
//             => If coap request fail -> MSG to main thread (mode 0) 
//             => Buzzer pressed -> MSG to main thread (mode 3) 
//      Mode 3 LOCKED: buzzer is locked
//             => If coap request fail -> MSG to main thread (mode 0) 
//             => buzzer reset received -> MSG to main thread (mode 2) 



void not_connected_mode(void){
    start_not_conn_blink_thread();
}

void pairing_mode(void){
    start_pairing_routine(&rcv_pid);
}

void normal_mode(void){
    enable_normal_mode();
}

void locked_mode(void){
    lock_buzzer();
}

void* main_routine(void* args){
    (void) args;
    

    
    
    msg_init_queue(rcv_queue, RCV_QUEUE_SIZE);

    msg_t msg;
    uint32_t mode = 0;

    printf("MAIN THREAD STARTING: %d\n",rcv_pid);
    init_buzzer(&rcv_pid);

    while(1){

        switch(mode){
            case MODE_NOT_CONNECTED: 
                not_connected_mode();
                break;
            case MODE_PAIRING: 
                pairing_mode();
                break;
            case MODE_NORMAL: 
                normal_mode();
                break;
            case MODE_LOCKED: 
                locked_mode();
                break;
            default:
                not_connected_mode();
        }

        msg_receive(&msg);
        printf("Received %" PRIu32 "\n", msg.content.value);
        mode = msg.content.value;

    }
}


int main(void)
{
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    
    // init_buzzer();
    rcv_pid = thread_create(main_routine_stack, sizeof(main_routine_stack),
                        THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_SLEEPING, main_routine, NULL, "main_thread");

    printf("MAIN: %d\n",rcv_pid);

    thread_wakeup(rcv_pid);
    

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    

    return 0;
}
