/* lib.c - Some basic library functions (printf, strlen, etc.)
 * vim:ts=4 noexpandtab
 */

#include "lib.h"
#define VIDEO 0xB8000
#define SAVED_VIDEO 0x100000
#define NUM_COLS 80
#define NUM_ROWS 25

static uint8_t ATTRIB;
static uint8_t cursor_x; // Keeps track of current column position
static uint32_t cursor_y; // Keeps track of cursor ofset from beginning of saved video memory
static char* video_mem = (char *)VIDEO; // Start of video memory (displayed)
static char* saved_video_mem = (char *)SAVED_VIDEO; // Start of saved video memory (stored)
static uint32_t screen_offset; // Number of saved offscreen lines
static uint8_t cursor_enabled; // 1 if cursor is displayed, 0 if not


/* Returns cursor column position. */
uint8_t
screen_x()
{
	return cursor_x;
}

/* Returns y position of cursor on screen. */
uint32_t
screen_y()
{
	return cursor_y - screen_offset;
}

/* Courtesy of http://wiki.osdev.org/Text_Mode_Cursor
 * void update_cursor()
 * by Dark Fiber
 * DESCRIPTION: Changes position of text-mode cursor.
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUES: none
 * SIDE EFFECTS: Enables/disables cursor and updates position.
 */
void update_cursor()
{
	uint8_t cur_CSR;
	// Checks if cursor is offscreen.
	if(screen_y() >= NUM_ROWS) {
		// Checks if cursor needs to be turned off.
		if(cursor_enabled != 0) {
			outb(0x0A, 0x3D4);
			cur_CSR = inb(0x3D5);
			outb(cur_CSR + 0x20, 0x3D5);
			cursor_enabled = 0;
		}
		return;
	}
	
	// Cursor is on screen, check if it needs to be turned on.
	if(cursor_enabled == 0) {
		outb(0x0A, 0x3D4);
		cur_CSR = inb(0x3D5);
		outb(cur_CSR - 0x20, 0x3D5);
		cursor_enabled = 1;
	}
	
	unsigned short position=(screen_y()*NUM_COLS) + screen_x();
 
	// cursor LOW port to vga INDEX register
	outb(0x0F, 0x3D4);
	outb((unsigned char)(position&0xFF), 0x3D5);
	// cursor HIGH port to vga INDEX register
	outb(0x0E, 0x3D4);
	outb((unsigned char )((position>>8)&0xFF), 0x3D5);
}

/*
 * DESCRIPTION: Initializes file-scope variables. 
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUES: none
 * SIDE EFFECTS: Sets video memory and saved video memory.
 */
void
screen_init()
{
	cursor_x = 0;
	cursor_y = 0;
	screen_offset = 0;
	cursor_enabled = 1;
	update_cursor();
	ATTRIB = 0x2; // Initialize to green on black.
	
	int32_t i;
	/* Go through all video memory and saved video memory and
	 * initialize to blank spaces. */
    for(i=0; i<NUM_ROWS*NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;
    }
	for(i=0; i<(0x400000-SAVED_VIDEO)/2; i++) {
		*(uint8_t *)(saved_video_mem + (i << 1)) = ' ';
        *(uint8_t *)(saved_video_mem + (i << 1) + 1) = ATTRIB;
	}
}


/* Credit given to Halo 2.
 * DESCRIPTION: Our blue screen of death implementation. Displayed when
 *				an exception occurs.
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUES: none
 * SIDE EFFECTS: Changes video memory.
 */
void
BSOD()
{
	int32_t i;
    for(i=0; i<NUM_ROWS*NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = 0x10; // Blue
	}
	ATTRIB = 0x17; // Blue on white
	cursor_y = screen_offset + 10;
	cursor_x = 10;
	puts("A total FU exception has occured at your location. All system");
	cursor_y++;
	cursor_x = 10;
	puts("functionality will be terminated.");
	cursor_y += 2;
	cursor_x = 10;
	puts("- ");
	puts("Press any key to power cycle the system. If system does not");
	cursor_y++;
	cursor_x = 12;
	puts("restart, scream at top of lungs and pound on keyboard.");
	cursor_y++;
	cursor_x = 10;
	puts("- ");
	puts("If you need to talk to a programmer press any other key.");
	cursor_y += 2;
	cursor_x = 26;
	puts("Press any key to continue..");
	cursor_y = screen_offset + 8;
	cursor_x = 20;
	ATTRIB = 0xF1;
	
	/* Disable cursor */
	if(cursor_enabled) {
		uint8_t cur_CSR;
		outb(0x0A, 0x3D4);
		cur_CSR = inb(0x3D5);
		outb(cur_CSR + 0x20, 0x3D5);
		cursor_enabled = 0;
	}
}


/*
 * DESCRIPTION: Clears video memory and sets cursor to top left of screen. 
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUES: none
 * SIDE EFFECTS: Changes video memory.
 */
void
clear(void)
{
    int32_t i;

	cursor_x = 0;
	cursor_y++;
	screen_offset = cursor_y; // Moves all saved video memory off screen.
	update_cursor();
	// Clears video memory and reflects this in saved video memory. 
    for(i=0; i<NUM_ROWS*NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;
		*(uint8_t *)(saved_video_mem + ((NUM_COLS*screen_offset + i) << 1)) = ' ';
        *(uint8_t *)(saved_video_mem + ((NUM_COLS*screen_offset + i) << 1) + 1) = ATTRIB;
    }
	

}


/*
 * DESCRIPTION: Scrolls up or down based on requested offset.
 * INPUTS: offset -- change in screen position
 * OUTPUTS: none
 * RETURN VALUES: none
 * SIDE EFFECTS: Changes displayed video memory.
 */
void
scroll(int32_t offset)
{
	// If offset is positive, it will scroll down.
	// If offset is negative, it will scroll up.
	
	// Prevents scrolling above saved video memory.
	if(offset + (int)screen_offset < 0) {
		if(screen_offset == 0)
			return;
		offset = 0 - (int)screen_offset; // Only scroll up enough to reach top, no farther.
	}
	// Prevents scrolling below cursor.
	else if((int)screen_offset + offset > cursor_y) {
		if(screen_offset == cursor_y)
			return;
		offset = cursor_y - (int)screen_offset; // Only scroll down enough to reach cursor.
	}
	
	screen_offset += offset;
	int i;
	// Copy saved video memory into displayed video memory.
	for(i=0; i<NUM_ROWS*NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) =
			*(uint8_t *)(saved_video_mem + ((NUM_COLS*screen_offset + i) << 1));
        *(uint8_t *)(video_mem + (i << 1) + 1) = 
			*(uint8_t *)(saved_video_mem + ((NUM_COLS*screen_offset + i) << 1) + 1);
	}
	update_cursor();
}

/* Changes font/background colors. */
void
update_color() {
	int32_t i;
	// Video memory uses pairs of bytes for each block on screen in text mode
	// We're only setting the second byte to change the font/background color.
	for(i=0; i<(0x400000-SAVED_VIDEO)/2; i++) {
        *(uint8_t *)(saved_video_mem + (i << 1) + 1) = ATTRIB;
	}
	scroll(0); // Update displayed video memory with new colors.
}

/* Changes to next font color. Called from terminal.c on F7 press. */
void
font_color() {
	uint8_t color = ATTRIB & 0x7;
	color = (color + 1) % 0x8;
	ATTRIB &= 0xF8;
	ATTRIB += color;
	update_color();
}


/* Changes to next background color. Called from terminal.c on F8 press. */
void
background_color() {
	uint8_t color = (ATTRIB & 0x70) >> 4;
	color = (color + 1) % 0x8;
	ATTRIB &= 0x8F;
	ATTRIB += (color << 4);
	update_color();
}


/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output.
 * */
int32_t
printf(int8_t *format, ...)
{
	/* Pointer to the format string */
	int8_t* buf = format;

	/* Stack pointer for the other parameters */
	int32_t* esp = (void *)&format;
	esp++;

	while(*buf != '\0') {
		switch(*buf) {
			case '%':
				{
					int32_t alternate = 0;
					buf++;

format_char_switch:
					/* Conversion specifiers */
					switch(*buf) {
						/* Print a literal '%' character */
						case '%':
							putc('%');
							break;

						/* Use alternate formatting */
						case '#':
							alternate = 1;
							buf++;
							/* Yes, I know gotos are bad.  This is the
							 * most elegant and general way to do this,
							 * IMHO. */
							goto format_char_switch;

						/* Print a number in hexadecimal form */
						case 'x':
							{
								int8_t conv_buf[64];
								if(alternate == 0) {
									itoa(*((uint32_t *)esp), conv_buf, 16);
									puts(conv_buf);
								} else {
									int32_t starting_index;
									int32_t i;
									itoa(*((uint32_t *)esp), &conv_buf[8], 16);
									i = starting_index = strlen(&conv_buf[8]);
									while(i < 8) {
										conv_buf[i] = '0';
										i++;
									}
									puts(&conv_buf[starting_index]);
								}
								esp++;
							}
							break;

						/* Print a number in unsigned int form */
						case 'u':
							{
								int8_t conv_buf[36];
								itoa(*((uint32_t *)esp), conv_buf, 10);
								puts(conv_buf);
								esp++;
							}
							break;

						/* Print a number in signed int form */
						case 'd':
							{
								int8_t conv_buf[36];
								int32_t value = *((int32_t *)esp);
								if(value < 0) {
									conv_buf[0] = '-';
									itoa(-value, &conv_buf[1], 10);
								} else {
									itoa(value, conv_buf, 10);
								}
								puts(conv_buf);
								esp++;
							}
							break;

						/* Print a single character */
						case 'c':
							putc( (uint8_t) *((int32_t *)esp) );
							esp++;
							break;

						/* Print a NULL-terminated string */
						case 's':
							puts( *((int8_t **)esp) );
							esp++;
							break;

						default:
							break;
					}

				}
				break;

			default:
				putc(*buf);
				break;
		}
		buf++;
	}

	return (buf - format);
}

/* Output a string to the console */
int32_t
puts(int8_t* s)
{
	register int32_t index = 0;
	while(s[index] != '\0') {
		putc(s[index]);
		index++;
	}

	return index;
}

void
putc(uint8_t c)
{
	if(c == '\n' || c == '\r') {
        cursor_y++;
        cursor_x=0;
    }
	// Handles backspace 
	else if(c == '\b') {
		if(cursor_x == 0) { // Moves to end of previous row
			cursor_y--;
			cursor_x = NUM_COLS - 1;
		} else {
			cursor_x--;
		}
		
		// Replace previous displayed character with a space.
		if(screen_y() < NUM_ROWS) {
			*(uint8_t *)(video_mem + ((NUM_COLS*screen_y() + screen_x()) << 1)) = ' ';
			*(uint8_t *)(video_mem + ((NUM_COLS*screen_y() + screen_x()) << 1) + 1) = ATTRIB;
		}
		*(uint8_t *)(saved_video_mem + ((NUM_COLS*cursor_y + cursor_x) << 1)) = ' ';
		*(uint8_t *)(saved_video_mem + ((NUM_COLS*cursor_y + cursor_x) << 1) + 1) = ATTRIB;
		
	}
	// Normal character
	else {
		if(screen_y() < NUM_ROWS) {
			*(uint8_t *)(video_mem + ((NUM_COLS*screen_y() + screen_x()) << 1)) = c;
			*(uint8_t *)(video_mem + ((NUM_COLS*screen_y() + screen_x()) << 1) + 1) = ATTRIB;
		}
		*(uint8_t *)(saved_video_mem + ((NUM_COLS*cursor_y + cursor_x) << 1)) = c;
        *(uint8_t *)(saved_video_mem + ((NUM_COLS*cursor_y + cursor_x) << 1) + 1) = ATTRIB;
        cursor_x++;
        cursor_y = (cursor_y + (cursor_x / NUM_COLS));
        cursor_x %= NUM_COLS;
    }
	
	if(screen_y() >= NUM_ROWS) // if cursor is offscreen
		scroll(screen_y() - NUM_ROWS + 1); // Scroll down so cursor is at bottom of screen.
	else
		update_cursor();
}

/* Convert a number to its ASCII representation, with base "radix" */
int8_t*
itoa(uint32_t value, int8_t* buf, int32_t radix)
{
	static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	int8_t *newbuf = buf;
	int32_t i;
	uint32_t newval = value;

	/* Special case for zero */
	if(value == 0) {
		buf[0]='0';
		buf[1]='\0';
		return buf;
	}

	/* Go through the number one place value at a time, and add the
	 * correct digit to "newbuf".  We actually add characters to the
	 * ASCII string from lowest place value to highest, which is the
	 * opposite of how the number should be printed.  We'll reverse the
	 * characters later. */
	while(newval > 0) {
		i = newval % radix;
		*newbuf = lookup[i];
		newbuf++;
		newval /= radix;
	}

	/* Add a terminating NULL */
	*newbuf = '\0';

	/* Reverse the string and return */
	return strrev(buf);
}

/* In-place string reversal */
int8_t*
strrev(int8_t* s)
{
	register int8_t tmp;
	register int32_t beg=0;
	register int32_t end=strlen(s) - 1;

	while(beg < end) {
		tmp = s[end];
		s[end] = s[beg];
		s[beg] = tmp;
		beg++;
		end--;
	}

	return s;
}

/* String length */
uint32_t
strlen(const int8_t* s)
{
	register uint32_t len = 0;
	while(s[len] != '\0')
		len++;

	return len;
}

/* Optimized memset */
void*
memset(void* s, int32_t c, uint32_t n)
{
	c &= 0xFF;
	asm volatile("                  \n\
			.memset_top:            \n\
			testl   %%ecx, %%ecx    \n\
			jz      .memset_done    \n\
			testl   $0x3, %%edi     \n\
			jz      .memset_aligned \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			subl    $1, %%ecx       \n\
			jmp     .memset_top     \n\
			.memset_aligned:        \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			movl    %%ecx, %%edx    \n\
			shrl    $2, %%ecx       \n\
			andl    $0x3, %%edx     \n\
			cld                     \n\
			rep     stosl           \n\
			.memset_bottom:         \n\
			testl   %%edx, %%edx    \n\
			jz      .memset_done    \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			subl    $1, %%edx       \n\
			jmp     .memset_bottom  \n\
			.memset_done:           \n\
			"
			:
			: "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
			: "edx", "memory", "cc"
			);

	return s;
}

/* Optimized memset_word */
void*
memset_word(void* s, int32_t c, uint32_t n)
{
	asm volatile("                  \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			cld                     \n\
			rep     stosw           \n\
			"
			:
			: "a"(c), "D"(s), "c"(n)
			: "edx", "memory", "cc"
			);

	return s;
}

/* Optimized memset_dword */
void*
memset_dword(void* s, int32_t c, uint32_t n)
{
	asm volatile("                  \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			cld                     \n\
			rep     stosl           \n\
			"
			:
			: "a"(c), "D"(s), "c"(n)
			: "edx", "memory", "cc"
			);

	return s;
}

/* Optimized memcpy */
void*
memcpy(void* dest, const void* src, uint32_t n)
{
	asm volatile("                  \n\
			.memcpy_top:            \n\
			testl   %%ecx, %%ecx    \n\
			jz      .memcpy_done    \n\
			testl   $0x3, %%edi     \n\
			jz      .memcpy_aligned \n\
			movb    (%%esi), %%al   \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			addl    $1, %%esi       \n\
			subl    $1, %%ecx       \n\
			jmp     .memcpy_top     \n\
			.memcpy_aligned:        \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			movl    %%ecx, %%edx    \n\
			shrl    $2, %%ecx       \n\
			andl    $0x3, %%edx     \n\
			cld                     \n\
			rep     movsl           \n\
			.memcpy_bottom:         \n\
			testl   %%edx, %%edx    \n\
			jz      .memcpy_done    \n\
			movb    (%%esi), %%al   \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			addl    $1, %%esi       \n\
			subl    $1, %%edx       \n\
			jmp     .memcpy_bottom  \n\
			.memcpy_done:           \n\
			"
			:
			: "S"(src), "D"(dest), "c"(n)
			: "eax", "edx", "memory", "cc"
			);

	return dest;
}

/* Optimized memmove (used for overlapping memory areas) */
void*
memmove(void* dest, const void* src, uint32_t n)
{
	asm volatile("                  \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			cld                     \n\
			cmp     %%edi, %%esi    \n\
			jae     .memmove_go     \n\
			leal    -1(%%esi, %%ecx), %%esi    \n\
			leal    -1(%%edi, %%ecx), %%edi    \n\
			std                     \n\
			.memmove_go:            \n\
			rep     movsb           \n\
			"
			:
			: "D"(dest), "S"(src), "c"(n)
			: "edx", "memory", "cc"
			);

	return dest;
}

/* Standard strncmp */
int32_t
strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
{
	int32_t i;
	for(i=0; i<n; i++) {
		if( (s1[i] != s2[i]) ||
				(s1[i] == '\0') /* || s2[i] == '\0' */ ) {

			/* The s2[i] == '\0' is unnecessary because of the short-circuit
			 * semantics of 'if' expressions in C.  If the first expression
			 * (s1[i] != s2[i]) evaluates to false, that is, if s1[i] ==
			 * s2[i], then we only need to test either s1[i] or s2[i] for
			 * '\0', since we know they are equal. */

			return s1[i] - s2[i];
		}
	}
	return 0;
}

/* Standard strcpy */
int8_t*
strcpy(int8_t* dest, const int8_t* src)
{
	int32_t i=0;
	while(src[i] != '\0') {
		dest[i] = src[i];
		i++;
	}

	dest[i] = '\0';
	return dest;
}

/* Standard strncpy */
int8_t*
strncpy(int8_t* dest, const int8_t* src, uint32_t n)
{
	int32_t i=0;
	while(src[i] != '\0' && i < n) {
		dest[i] = src[i];
		i++;
	}

	while(i < n) {
		dest[i] = '\0';
		i++;
	}

	return dest;
}

void
test_interrupts(void)
{
	int32_t i;
	for (i=0; i < NUM_ROWS*NUM_COLS; i++) {
		video_mem[i<<1]++;
	}
}
