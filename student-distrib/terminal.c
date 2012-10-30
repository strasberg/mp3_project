#include "terminal.h"
#include "lib.h"

#define VIDEO 0xB8000
#define SAVED_VIDEO 0xB9000

static uint8_t buffer[1024];

/* Keeps track of # of elements in buffer */
static int16_t line_pos;

static volatile int8_t enter_pressed;
static int8_t shift;
static int8_t caps_lock;
static int8_t ctrl;

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
	shift = 0;
	caps_lock = 0;
	ctrl = 0;
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
		i++;
	}

	line_pos = 0;
	return rtn_cnt;
}

int32_t
terminal_write(const uint8_t* buf, int32_t cnt)
{
	int8_t s[1024];
	
	int i;
	for(i = 0; i < cnt; i++)
		s[i] = buf[i];
	s[cnt] = '\0';

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
static uint8_t chars[128] =
{
    0,  0, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  0,			/* Tab */
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

static uint8_t shift_chars[128] =
{
    0,  0, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
  '(', ')', '_', '+', 0,	/* Backspace */
  0,			/* Tab */
  'Q', 'W', 'E', 'R',	/* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
 '"', '~',   0,		/* Left shift */
 '|', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
  'M', '<', '>', '?',   0,				/* Right shift */
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
keyboard_input(uint8_t key)
{
	//printf("%x ",key);
	key &= 0xFF;

	switch(key) {
		case 0x1D:
		ctrl = 1;
		return;
		
		case 0x9D:
		ctrl = 0;
		return;
		
		case 0x48:
		scroll(-1);
		return;
		
		case 0x50:
		scroll(1);
		return;
		
		case 0x3A:
		caps_lock = 1 - caps_lock;
		return;
	
		case 0x2A:
		shift = 1;
		return;
		
		case 0xAA:
		shift = 0;
		return;
		
		case 0x36:
		shift = 1;
		return;
		
		case 0xB6:
		shift = 0;
		return;
		
		case 0xF:
		if(shift)
			background_color();
		else
			font_color();
		return;
	}
	
	uint8_t kbd_data;
	
	if(shift || caps_lock)
		kbd_data = shift_chars[key];
	else
		kbd_data = chars[key];
		
	if(ctrl) {
		if(kbd_data == 'l') {
			clear();
			line_pos = 0;
		}
		return;
	}
		
	if((0x80 & key) != 0)
		return;
	
	if(kbd_data != 0) {
		if(kbd_data == '\b') {
			if(line_pos == 0)
				return;
			line_pos--;
		} else if(kbd_data == '\n') {
			buffer[line_pos] = '\n';
			enter_pressed = 1;
			line_pos = 0;
		} else {
			if(line_pos >= 1024)
				return;
			buffer[line_pos] = kbd_data;
			line_pos++;
		}
		putc(kbd_data);
	}
}


