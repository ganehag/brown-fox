#ifndef gdefsh
#define gdefsh

extern volatile uint8_t rtcc;
void rtc_alarm(uint16_t ticks);
uint8_t chk_alarm(void);

void cb(void);

#endif