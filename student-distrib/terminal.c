#include "terminal.h"
#include "lib.h"

#define VIDEO 0xB8000
#define SAVED_VIDEO 0xB9000

static uint8_t typed[1024];

/* Keeps track of # of elements in typed */
static int16_t line_pos;

// Active high
static volatile int8_t enter_pressed;
static int8_t shift;
static int8_t caps_lock;
static int8_t ctrl;

static int8_t reading; // 1 if read fn. is running, 0 otherwise

/* 
 * terminal_open
 *   DESCRIPTION: Initializes file-scope variables
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: terminal ready for use
 */
int32_t
terminal_open()
{
	screen_init();
	line_pos = 0;
	shift = 0;
	caps_lock = 0;
	ctrl = 0;
	reading = 0;
	return 0;
}


/* 
 * terminal_read
 *   DESCRIPTION: Fills in buffer with all keyboard presses once Enter has been pressed.
 *   INPUTS: buf -- character array to be filled in
 *           cnt -- number of characters requested
 *   OUTPUTS: none
 *   RETURN VALUE: number of characters written to buffer
 *   SIDE EFFECTS: none
 */
int32_t
terminal_read(uint8_t* buf, int32_t cnt)
{
	reading = 1;
	line_pos = 0; // Nothing in the typed so far.

	enter_pressed = 0;
	
	/* Wait until Enter has been pressed. */
	while(!enter_pressed) {}
	
	int32_t rtn_cnt = 0; // Number of characters actually written to buffer.
	while((typed[rtn_cnt] != '\n') && (rtn_cnt < cnt))
	{
		buf[rtn_cnt] = typed[rtn_cnt];
		rtn_cnt++;
	}

	return rtn_cnt;
}


/* 
 * terminal_write
 *   DESCRIPTION: Print cnt # of characters in buf to screen.
 *   INPUTS: buf -- character array to be printed
 *           cnt -- number of characters requested to be printed
 *   OUTPUTS: none
 *   RETURN VALUE: number of characters written to screen
 *   SIDE EFFECTS: none
 */
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

/*
 * terminal_close
 *   DESCRIPTION: Does nothing
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: wastes your time
 */
int32_t
terminal_close()
{
	return 0;
}

/* Stolen from www.osdever.net/bkerndev/Docs/keyboard.htm */
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
  '(', ')', '_', '+', '\b',	/* Backspace */
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


/*
 * keyboard_input
 *   DESCRIPTION: Processes all keyboard input. Letters get printed to screen while read fn. is
 *                running, some other keys have special functions.
 *   INPUTS: key -- 8 bit scancode passed in from keyboard handler
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
keyboard_input(uint8_t key)
{
    // Only process keyboard input if read fn. is executing
	if(!reading)
		return;

	switch(key) {
        // Ctrl pressed
		case 0x1D:
		ctrl = 1;
		return;
		
        // Ctrl realeased
		case 0x9D:
		ctrl = 0;
		return;
		
        // Up arrow pressed
		case 0x48:
		scroll(-1);
		return;
		
        // Down arrow pressed
		case 0x50:
		scroll(1);
		return;
		
        // Page Up pressed
		case 0x49:
		scroll(-12);
		return;
		
        // Page Down pressed
		case 0x51:
		scroll(12);
		return;
		
        // Caps Lock pressed/released
		case 0x3A:
		caps_lock = 1 - caps_lock;
		return;
	
        // Left shift pressed
		case 0x2A:
		shift = 1;
		return;
		
        // Left shift released
		case 0xAA:
		shift = 0;
		return;
        
        // Right shift pressed
		case 0x36:
		shift = 1;
		return;
		
        // Right shift released
		case 0xB6:
		shift = 0;
		return;
		
        // F7 pressed
		case 0x41:
		font_color();
		return;
		
        // F8 pressed
		case 0x42:
		background_color();
		return;
	}
	
	uint8_t kbd_data;
	
    // Decide which kbd array to use.
	if(shift || caps_lock)
		kbd_data = shift_chars[key];
	else
		kbd_data = chars[key];
		
	if(ctrl) {
        // Clear screen on <Ctrl + l>
		if(kbd_data == 'l') {
			clear();
			line_pos = 0;
		}
		return;
	}
	
    // Return if scancode is for key release.
	if((0x80 & key) != 0)
		return;
	
    // Only process valid characters.
	if(kbd_data != 0) {
		if(kbd_data == '\b') {
            // Don't allow backspacing farther than beginning of typed buffer.
			if(line_pos == 0)
				return;
			line_pos--;
		} else if(kbd_data == '\n') {
			reading = 0;
			typed[line_pos] = '\n';
			enter_pressed = 1;
		}
        // Printable characters
        else {
            // Only allow 1024 characters in buffer.
			if(line_pos >= 10)
				return;
			typed[line_pos] = kbd_data;
			line_pos++;
		}
		putc(kbd_data);
	}
}


