#ifndef __DISPLAY4BIT_H
#define __DISPLAY4BIT_H

#define	LCD_DB4 21
#define	LCD_DB5 22
#define	LCD_DB6 23
#define	LCD_DB7 24

#define	LCD_E 25
#define	LCD_RS 26
#define	LCD_RW 27

void lcd_init_4bit(void);
void lcd_putchar( char CHAR);
void command( char CHAR);
void lcd_putsf( char *data);
void ds_delay(int delay);

#endif
