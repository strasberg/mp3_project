/* rtc.h */
#ifndef _RTC_H
#define _RTC_H

#define	 RTC_CMD	0x70
#define	 RTC_DATA	0x71

extern int rtc_open();
extern int rtc_write(int freq);
extern int rtc_read();
extern int rtc_close();
//extern char rtc_intr(char int_data);

#endif
