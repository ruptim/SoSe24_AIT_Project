#ifndef GAMESHOW_BUZZER_H
#define GAMESHOW_BUZZER_H

#include <stdlib.h>
#include <stdint.h>

#include "thread.h"
 
void get_iso8601_time(char *buffer, size_t buffer_size);

int get_time(int argc, char **argv);

int init_buzzer(kernel_pid_t* main_thread_pid);
 
void init_buzzer_resources(kernel_pid_t* main_thread_pid);

void start_not_conn_blink_thread(void);

void start_pairing_routine(kernel_pid_t* main_thread_pid);

void start_heartbeat_routine(kernel_pid_t *main_thread_pid);

void enable_normal_mode(void);
void disble_normal_mode(void);

void lock_buzzer(void);
void unlock_buzzer(void);


void enable_not_connected_mode(void);
void set_connection_status(bool connected);


void send_buzzer_pressed(kernel_pid_t* main_thread_pid);

#endif /* GAMESHOW_BUZZER_H */