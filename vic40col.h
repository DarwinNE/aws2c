#ifndef __VIC40COL_H__
#define __VIC40COL_H__

extern void initGraphic(void);
extern void normalText(void);

extern void clrscr(void);
extern void negative(void);
extern void positive(void);

extern void __fastcall__ putc40ch(char c);
extern void __fastcall__ puts40ch(char* s);
extern void __fastcall__ gets40ch(char* s, unsigned char len);
extern unsigned char __fastcall__ cgetc40ch(void);

#endif