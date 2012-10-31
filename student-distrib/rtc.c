/*rtc.c*/
#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "flags.h"

int rtc_open()
{
	cli();
	rtc_uie=1;
	sti();
	//Select Reg A and write 2Hz Freq
	outb(0x8A,RTC_CMD);
	outb(0x2f,RTC_DATA);
	if(rtc_uie!=0)
		return -1;
		
	cli();
	rtc_uie=1;
	sti();
	//Select Reg B and enable Periodic Interrupt and Update Ended Interrupt
	outb(0x8B,RTC_CMD);	
	outb(0x50,RTC_DATA);
	if(rtc_uie==0)
		return 0;
	else
		return -1;
}
int rtc_write(int freq)
{
	if(freq<=10)
	{
	//Calculate new frequency term
		char temp=0xff;
		temp=temp^(freq-1);
		temp=temp&0x0f;
		temp=temp|0x20;
	//Set uie flag for error checking
		rtc_uie=1;
	//Select Reg A and write new Freq
		outb(0x8A,RTC_CMD);
		outb(temp,RTC_DATA);
		if(rtc_uie==0)
			return 0;
		else
			return -1;
	}
	else 
		return -1;
}
int rtc_read()
{
	cli();
	rtc_pie=1;
	sti();
	while(rtc_pie==1)
	{
		//spin till change
	}	
	return 0;
}
int rtc_close()
{
	cli();
	rtc_uie=1;
	sti();
	//Select Reg A and Disable Oscillator
	outb(0x8A,RTC_CMD);
	outb(0x00,RTC_DATA);
	if(rtc_uie!=0)
		return -1;
		
	cli();
	rtc_uie=1;
	sti();
	//Select Reg B and Disable Everything
	outb(0x8B,RTC_CMD);	
	outb(0x00,RTC_DATA);
	if(rtc_uie==0)
		return 0;
	else
		return -1;
}

