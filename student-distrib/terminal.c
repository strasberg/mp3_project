#include "terminal.h"
#include "lib.h"

#define VIDEO 0xB8000
#define SAVED_VIDEO 0xB9000

static uint8_t buffer[1024];

/* Keeps track of # of elements in buffer */
static int16_t line_pos;

static volatile int8_t enter_pressed;


/* 
 * terminal_open
 *   DESCRIPTION: Initializes buffer, gets current cursor position.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: terminal ready for use
 */
int32_t
terminal_open()
{
	line_pos = 0;
	return 0;
}

int32_t
terminal_read(uint8_t* buf, int32_t cnt)
{
	line_pos = 0;
	enter_pressed = 0;
	
	/* Wait until Enter has been pressed. */
	while(!enter_pressed) {}
	
	int32_t rtn_cnt = 0;
	int i = 0;
	while((buffer[i] != '\n') && (i < cnt))
	{
		buf[i] = buffer[i];
		rtn_cnt++;
	}

	line_pos = 0;
	return rtn_cnt;
}

int32_t
terminal_write(const uint8_t* buf, int32_t cnt)
{
	int8_t s[cnt + 1];
	int i;
	for(i = 0; i < cnt; i++)
		s[i] = buf[i];
	s[cnt] = '\0';
	//cursor_x += cnt;
	//cursor_y = 
	return puts(s);
}

int32_t
terminal_close()
{
	return 0;
}

/* KBDUS means US Keyboard Layout. This is a scancode table
*  used to layout a standard US keyboard. I have left some
*  comments in to give you an idea of what key is what, even
*  though I set it's array index to 0. You can change that to
*  whatever you want using a macro, if you wish! */
static uint8_t kbdus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};		

void
keyboard_input(uint16_t key)
{
	const uint8_t kbd_data = kbdus[key];
	if(kbd_data == 'w')
		scroll(-1);
	if(kbd_data == 's')
		scroll(1);
	terminal_write(&kbd_data,1);
}


