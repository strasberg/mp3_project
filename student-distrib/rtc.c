/*rtc.c*/
#include "rtc.h"
#include "lib.h"
#include "i8259.h"
//Local Flags
volatile int rtc_intr_recieved;
volatile int rtc_pie;
volatile int rtc_uie;
//Freqeuncy Decoder
int rtc_freq;
int freq[10]={2,4,8,16,32,64,128,256,512,1024};
/* RTC_INTR
 *Purpose:	Function that allows for external manipulation of local flags
 *Action:	Temp should be contents read from Reg C of RTC on interrupt; 
 *			Function should translate this and clear the appropriate flags
 *Note: 	Ideally should only be used by RTC handler
*/
void rtc_intr(uint8_t temp)
{
	if(temp==0xC0)
	{
		rtc_pie=0;
	}
	else if(temp==0x90)
	{
		rtc_uie=0;
	}
	else if(temp==(0x90|0xC0))
	{
		rtc_uie=0;
		rtc_pie=0;
	}
	rtc_intr_recieved=0;
}
/*RTC_OPEN
*Purpose:	Initialize the RTC w/ a default freqency of 2Hz and enabling PIE & UIE
*Action: 	Writes to RTC_CMD and RTC_DATA ports setting the appropriate bits high on Reg A and B
*/
int rtc_open()
{
	//Select Reg A and write 2Hz Freq
	outb(0x8A,RTC_CMD);
	outb(0x2f,RTC_DATA);
	
	rtc_freq=2;
	//Select Reg B and enable Periodic Interrupt and Update Ended Interrupt
	outb(0x8B,RTC_CMD);	
	outb(0x50,RTC_DATA);
	return 0;
}
/*RTC_WRITE
*Purpose:	Write a new frequency to Reg B based on 2^count
*Action:	Translates cnt into appropriate byte and write it to Reg B, 
*			Check for UIE to indicate successful write
*Note:		buf is not used; Arguments are kept the same to match systemcall write
*/
int rtc_write(uint8_t* buf, int32_t cnt)
{
	int cntr;
	cntr=0;
	if(cnt<=10&&cnt>0)
	{
	//Update rtc_freq;
		rtc_freq=freq[cnt-1];
	//Calculate new frequency term
		char temp=0xff;
		temp=temp^(cnt-1);
		temp=temp&0x0f;
		temp=temp|0x20;
	//Set uie flag for error checking
		rtc_uie=1;
	//Select Reg A and write new Freq
		outb(0x8A,RTC_CMD);
		outb(temp,RTC_DATA);
	//Wait for fixed amt of time for UIE
		rtc_intr_recieved=1;
		while(rtc_uie==1&&cntr<rtc_freq)
		{
			while(rtc_intr_recieved==1){}
				rtc_intr_recieved=1;
				cntr++;
		}
		if(rtc_uie==0)
			return 0;
		else
			return -1;
	}
	else 
		return -1;
}
/*RTC_Read
*Purpose: 	Read from the RTC, Return 0 after PIE
*Action: 	Returns 0 after PIE
*Note: 		buf & cnt is not used; Arguments are kept the same to match systemcall read
*/
int rtc_read(uint8_t* buf, int32_t cnt)
{
	rtc_pie=1;
	while(rtc_pie==1)
	{
		//spin till change
	}	
	return 0;
}
/*RTC_Close
*Purpose: 	Disables the RTC for debuggin
*Action:	Writes to Reg A and B; disabling everything
*/
int rtc_close()
{
	//Select Reg A and Disable Oscillator
	outb(0x8A,RTC_CMD);
	outb(0x00,RTC_DATA);

	//Select Reg B and Disable Everything
	outb(0x8B,RTC_CMD);	
	outb(0x00,RTC_DATA);
	return 0;
}

