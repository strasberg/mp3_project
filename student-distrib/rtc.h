/* rtc.h */
#ifndef _RTC_H
#define _RTC_H

#include "types.h"

#define	 RTC_CMD	0x70
#define	 RTC_DATA	0x71

extern int rtc_open();
extern int rtc_write(uint8_t* buf, int32_t cnt);
extern int rtc_read(uint8_t* buf, int32_t cnt);
extern int rtc_close();
//extern char rtc_intr(char int_data);

extern void rtc_intr(uint8_t temp);

#endif
