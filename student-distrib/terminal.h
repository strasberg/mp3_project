#ifndef TERMINAL_H
#define TERMINAL_H

#include "types.h"

/* Does nothing, returns 0. */
extern int32_t terminal_open();
/* Returns cnt chars (or up to '\n') after Enter has been pressed. */
extern int32_t terminal_read(uint8_t* buf, int32_t cnt);
/* Gets keyboard input and modifies the buffer and the screen. */
extern int32_t terminal_write(const uint8_t* buf, int32_t cnt);
/* Does nothing, returns 0. */
extern int32_t terminal_close();
/* Translates keyboard input into letters and calls terminal_write. */
extern void keyboard_input(uint8_t key);

#endif
