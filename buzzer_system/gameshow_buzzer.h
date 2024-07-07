#ifndef GAMESHOW_BUZZER_H
#define GAMESHOW_BUZZER_H

#include <stdlib.h>
#include <stdint.h>

void get_iso8601_time(char *buffer, size_t buffer_size);

int get_time(int argc, char **argv);

int init_buzzer(void);
 
void init_buzzer_resources(void);

#endif /* GAMESHOW_BUZZER_H */