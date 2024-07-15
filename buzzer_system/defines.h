#ifndef DEFINES_H
#define DEFINES_H

#define TIME_STRING_LEN 30
#define MAX_BUZZER_ID_LEN 32

#define HEARTBEAT_SLEEP_DURATION_MS 3000


#define CONFIG_URI_MAX 128
#define BUZZER_RESET_URI "/buzzer/reset_buzzer"
#define BUZZER_SERVER_HEARTBEAT_URI "/b/heartbeat"
#define BUZZER_SERVER_PRESSED_URI "/b/pressed"
#define BUZZER_SERVER_REGISTER_URI "/b/register"
#define BUZZER_SERVER_RE_REGISTER_OK "0"

enum MODES{
    EVENT_NOT_CONNECTED,
    EVENT_START_PAIRING,
    EVENT_CONNECTED,
    EVENT_ENABLE_NORMAL,
    EVENT_ENABLE_LOCKED,
    EVENT_BUZZER_PRESSED
};
 

#endif /* DEFINES_H */