#ifndef _COMMANDERX16_H_
#define _COMMANDERX16_H_

void putc_x16(char c);
void puts_x16(char *s);
char *gets_x16(char *buffer, unsigned int size);
char getc_x16(void);
void init_x16(void);
void console_paging(void);


#define console_init() asm ("jsr $FEDB")
#define screen_set_mode() asm("jsr $FF5F");
#define console_set_paging_message() asm("jsr $FED5");

#define COLOR_GREEN    0x1E
#define COLOR_RED      0x1C
#define COLOR_CYAN     0x9F
#define COLOR_BLUE     0x1F
#define COLOR_YELLOW   0x9E
#define COLOR_PINK     0x96
#define COLOR_BLACK    0x90

#define ATTR_UNDERLINE 0x04
#define ATTR_BOLD      0x06
#define ATTR_ITALICS   0x0B
#define ATTR_OUTLINE   0x0C
#define ATTR_RESET     0x92


#endif