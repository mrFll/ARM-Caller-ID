#include "lpc17xx.h"                              /* LPC17xx definitions    */
#include "display4bit.h"



//----------------------------------

#define   PIO_SetOutput	   LPC_GPIO1->FIOSET 
#define   PIO_ClearOutput  LPC_GPIO1->FIOCLR
#define   PIO_CfgOutput    LPC_GPIO1->FIODIR


#define d1 300
#define d2 50000


//-----------------------------
void ds_delay(int delay){
     long int i;
     for(i=0;i<=delay;i++) ;
}
//-----------------------------
void lcd_init_4bit(void){
	
	PIO_CfgOutput =   ((1<<LCD_RS)|(1<<LCD_E)|(1<<LCD_RW)|(1<<LCD_DB4)|(1<<LCD_DB5)|(1<<LCD_DB6)|(1<<LCD_DB7));
	PIO_ClearOutput = ((1<<LCD_RS)|(1<<LCD_E)|(1<<LCD_RW)|(1<<LCD_DB4)|(1<<LCD_DB5)|(1<<LCD_DB6)|(1<<LCD_DB7));
	
   PIO_ClearOutput = (1<<LCD_RS);  // for command
	 PIO_ClearOutput = (1<<LCD_RW);
	 PIO_SetOutput = 0x03 << LCD_DB4;
	 PIO_SetOutput = 1<<LCD_E ;
	 ds_delay(d1);
	 PIO_ClearOutput = 1<<LCD_E ;
	 ds_delay(d2);
	 PIO_ClearOutput = 0x03 << LCD_DB4;

	 PIO_SetOutput = 0x03 << LCD_DB4;
	 PIO_SetOutput = 1<<LCD_E ;
	 ds_delay(d1);
	 PIO_ClearOutput = 1<<LCD_E ;
	 ds_delay(d2);
	 PIO_ClearOutput = 0x03 << LCD_DB4;

	 PIO_SetOutput = 0x02 << LCD_DB4;
	 PIO_SetOutput = 1<<LCD_E ;
	 ds_delay(d1);
	 PIO_ClearOutput = 1<<LCD_E ;
	 ds_delay(d2);
	 PIO_ClearOutput = 0x02 << LCD_DB4;
     /////////////////////////////
   PIO_SetOutput = 0x03 << LCD_DB4;
	 PIO_SetOutput = 1<<LCD_E ;
	 ds_delay(d1);
	 PIO_ClearOutput = 1<<LCD_E ;
	 ds_delay(d2);
	 PIO_ClearOutput = 0x03 << LCD_DB4;

	 PIO_SetOutput = 0x03 << LCD_DB4;
	 PIO_SetOutput = 1<<LCD_E ;
	 ds_delay(d1);
	 PIO_ClearOutput = 1<<LCD_E ;
	 ds_delay(d2);
	 PIO_ClearOutput = 0x03 << LCD_DB4;

	 PIO_SetOutput = 0x02 << LCD_DB4;
	 PIO_SetOutput = 1<<LCD_E ;
	 ds_delay(d1);
	 PIO_ClearOutput = 1<<LCD_E ;
	 ds_delay(d2);
	 
   PIO_ClearOutput = 0x02 << LCD_DB4;
	 	//MAT 28
	command(0x28);
	//LCD CLEAR
	command(0x01);
	//ON DISPLAY & CURSOR-OFF
	command(0x0C);
	//RIGHT	 LCD
	command(0x06);
	//X_Y CONFIG X=0;Y=0
	command(0x80);
}
//-----------------------------
void lcd_putchar( char CHAR){

     PIO_SetOutput = (1<<LCD_RS);  // for character
     PIO_ClearOutput = (1<<LCD_RW);
     PIO_SetOutput = ((CHAR&0XF0)>>4)<< LCD_DB4;
     PIO_SetOutput = 1<<LCD_E ;
     ds_delay(d1);
     PIO_ClearOutput = 1<<LCD_E ;
     ds_delay(d2);
     PIO_ClearOutput = ((CHAR&0XF0)>>4)<< LCD_DB4;

     PIO_SetOutput = (CHAR&0X0F)<< LCD_DB4;
     PIO_SetOutput = 1<<LCD_E ;
     ds_delay(d1);
     PIO_ClearOutput = 1<<LCD_E ;
     ds_delay(d2);
     PIO_ClearOutput = (CHAR&0X0F)<< LCD_DB4;
}
//-----------------------------
void command( char CHAR){

   PIO_ClearOutput = (1<<LCD_RS);  // for command
	 PIO_ClearOutput = (1<<LCD_RW);
	 PIO_SetOutput = ((CHAR&0XF0)>>4)<< LCD_DB4;
	 PIO_SetOutput = 1<<LCD_E ;
	 ds_delay(d1);
	 PIO_ClearOutput = 1<<LCD_E ;
	 ds_delay(d2);
	 PIO_ClearOutput = ((CHAR&0XF0)>>4)<< LCD_DB4;

	 PIO_SetOutput = (CHAR&0X0F)<< LCD_DB4;
	 PIO_SetOutput = 1<<LCD_E ;
	 ds_delay(d1);
	 PIO_ClearOutput = 1<<LCD_E ;
	 ds_delay(d2);
	 PIO_ClearOutput = (CHAR&0X0F)<< LCD_DB4;
}
//-----------------------------
void lcd_putsf( char *data){
	long int i;
	for(i=0;data[i];i++) lcd_putchar(data[i]);
}
