/*
 * Copyright (C) 2024 ruptim
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     buzzer system
 * @{
 *
 * @file
 * @brief       a distributed buzzer system
 *
 * @author      ruptim <timonrupelt@gmx.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>


#include "shell.h"


#include "led.h"
#include "periph/gpio.h"
#include "net/sntp.h"
#include "net/wifi.h"
#include "ztimer.h"
#include "mutex.h"

#ifdef MODULE_NETIF
#include "net/gnrc/pktdump.h"
#include "net/gnrc.h"
#endif

#define DEBOUNCE_DELAY_MS 20

ztimer_now_t btn_debounce_ts;
ztimer_t timer;
mutex_t led1_mutex = MUTEX_INIT;

static void btn_callback(void *arg){
    unsigned int *led1 = (unsigned int*) arg;

     unsigned int btn = GPIO_PIN(PORT_D, 2);
    
    ztimer_acquire(ZTIMER_MSEC);
    ztimer_now_t cur_ts = ztimer_now(ZTIMER_MSEC);
    ztimer_release(ZTIMER_MSEC);

    if(cur_ts - btn_debounce_ts >= DEBOUNCE_DELAY_MS){
        if (*led1 != GPIO_UNDEF)
        {
            mutex_lock(&led1_mutex);
            gpio_toggle(*led1);
            int v = gpio_read(btn);
            printf("Button: %d\n",v);
            // printf("PRESSED!\n");
            mutex_unlock(&led1_mutex);
        }
        btn_debounce_ts = cur_ts;
    }

}

void timer_callback(void *arg){
    (void) arg;

    mutex_lock(&led1_mutex);
    LED_TOGGLE(0);
    mutex_unlock(&led1_mutex);

    // ztimer_set(ZTIMER_MSEC,&timer,1000);

}

static int hello_world(int argc, char **argv) {
    /* Suppress compiler errors */
    (void)argc;
    (void)argv;
    printf("hello world!\n");

    /* toggle led1 */
    unsigned int led1 = GPIO_PIN(1,0);
    if (led1 != GPIO_UNDEF){
        int val = gpio_read(led1);

        mutex_lock(&led1_mutex);
        gpio_write(led1,!(bool) val);
        mutex_unlock(&led1_mutex);
    }

    /* read button*/
    unsigned int btn = GPIO_PIN(2,13);
    if (btn != GPIO_UNDEF){
        int val = gpio_read(btn);
        printf("Button: %s\n",val==0?"LOW":"HIGH");
    }

    uint64_t usec = sntp_get_unix_usec();
    printf("USEC: %lld \n",usec);

    return 0;
}

const shell_command_t shell_commands[] = {
    {"hello", "prints hello world", hello_world},
    { NULL, NULL, NULL }
};




int main(void)
{
    
    // unsigned int led1 = GPIO_PIN(1,0);
    // unsigned int btn = GPIO_PIN(2,13);

    // unsigned int btn = BTN0_PIN;
    unsigned int led1 = LED0_PIN;

    // unsigned int btn = GPIO_PIN(PORT_F,13);

    unsigned int btn = GPIO_PIN(PORT_D, 2);
    // gpio_init(btn, GPIO_IN_PU);
    // unsigned int led1 = GPIO_PIN(PORT_E, 18);
    // gpio_init(led1, GPIO_OUT);

    if (btn != GPIO_UNDEF){
        gpio_init_int(btn,GPIO_IN,GPIO_FALLING,btn_callback,(void*) &led1);
        
        ztimer_acquire(ZTIMER_MSEC);
        btn_debounce_ts = ztimer_now(ZTIMER_MSEC);
        ztimer_release(ZTIMER_MSEC);
    }

    timer.callback=timer_callback;        
    ztimer_set(ZTIMER_MSEC,&timer,1000);


    (void) puts("Welcome to RIOT!");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}