
#include <stdio.h>
#include "RTL.h"
#include "lpc17xx.h"
#include "string.h"

#include "Net_Config.h"

#include "RTL.h"                      
#include "File_Config.h"
#include "Serial.h"


struct __FILE { int handle; };
/*
FILE __stdout;
FILE __stdin;
*/

#define  LED1  (1<<25)  //PORT1.25
#define  LED2  (1<<26)  //PORT1.26
#define  LED3  (1<<27)  //PORT1.27
#define  LED4  (1<<28)  //PORT1.28

#define  SET_LED1 (LPC_GPIO1->FIOSET = LED1)
#define  SET_LED3 (LPC_GPIO1->FIOSET = LED3)
#define  SET_LED4 (LPC_GPIO1->FIOSET = LED4)

#define  CLR_LED1 (LPC_GPIO1->FIOCLR = LED1)
#define  CLR_LED3 (LPC_GPIO1->FIOCLR = LED3)
#define  CLR_LED4 (LPC_GPIO1->FIOCLR = LED4)



BOOL LEDrun;
BOOL show_update;
BOOL tick;
U32  dhcp_tout;
U8   lcd_text[2][16+1] = {" ",                /* Buffer for LCD text         */
                          "Waiting for DHCP"};

extern LOCALM localm[];                       /* Local Machine Settings      */
#define MY_IP localm[NETIF_ETH].IpAdr
#define DHCP_TOUT   50                        /* DHCP timeout 5 seconds      */
													


static void init_io (void);

/*--------------------------- init ------------------------------------------*/
static void init () {
  /* Add System initialisation code here */ 

  init_io ();
  init_TcpNet ();

  /* Setup and enable the SysTick timer for 100ms. */
  SysTick->LOAD = (SystemCoreClock / 10) - 1;
  SysTick->CTRL = 0x05;
}

/*--------------------------- timer_poll ------------------------------------*/
static void timer_poll () {
  /* System tick timer running in poll mode */

  if (SysTick->CTRL & 0x10000) {
    /* Timer tick every 100 ms */
    timer_tick ();
    tick = __TRUE;
  }
}

/*--------------------------- init_io ---------------------------------------*/
static void init_io () {

 /* Configure the GPIO for LEDs. */
  LPC_GPIO1->FIODIR =  LED1 | LED2 |LED3 | LED4;           /* LEDs PORT1 25...28 are Output */
  LPC_GPIO1->FIOCLR =  LED1 | LED2 | LED3 | LED4;		       /* turn off all the LEDs */

  /* Configure the GPIO for Push Buttons */
  LPC_PINCON->PINSEL4 &= ~((3<<20)|(3<<22)|(3<<24));
  LPC_GPIO2->FIODIR   &= ~((1<<10)|(1<<11)|(3<<12));
  
  /* Configure AD0.5 input. */
 	LPC_SC->PCONP |= (1 << 12);     // Enable power to AD block 
  LPC_PINCON->PINSEL3 &= ~(3UL<<30);   /* P1.31 is GPIO */
  LPC_PINCON->PINSEL3 |=  (3UL<<30);   /* P1.31 is AD0.5 */
	
  LPC_ADC->ADCR |= (1<<5);       //select AD0.5  0x1  0x2 0x4  0x8 1000
  LPC_ADC->ADCR |= 1<<8;     //  	ADCLK is 12.5 MHz  1 0000 0000
  //LPC_ADC->ADCR |= 0x10000;    // BURST bit   1 << 16
  LPC_ADC->ADCR |= 0x200000;   //Power up,    1 << 21
	
}


//
void LED_out (U32 val) {
  U32 led_pos[4] = { LED1, LED2, LED3, LED4 };
  U32 i;

  for (i = 0; i < 4; i++) {
    if (val & (1<<i)) {
       LPC_GPIO1->FIOSET = led_pos[i];
    }
    else {
       LPC_GPIO1->FIOCLR = led_pos[i];
   }
 }
}

//
U8 get_button (void) {
	
 if ((LPC_GPIO2->FIOPIN & (1 << 11)) == 0) {
    /* Key1 */
    return 0x00;
  }
  if ((LPC_GPIO2->FIOPIN & (1 << 12)) == 0) {
    /* Key2 */
    return 0x01;
  }
}


//
static void dhcp_check () {
  /* Monitor DHCP IP address assignment. */

  if (tick == __FALSE || dhcp_tout == 0) {
    return;
  }
//  MY_IP[0] = 192; MY_IP[1] = 168 ; MY_IP[2] = 1; MY_IP[3] = 98;
  if (mem_test (&MY_IP, 0, IP_ADRLEN) == __FALSE && !(dhcp_tout & 0x80000000)) {
    /* Success, DHCP has already got the IP address. */
    dhcp_tout = 0;
    sprintf((char *)lcd_text[0]," IP address:");
    sprintf((char *)lcd_text[1]," %d.%d.%d.%d", MY_IP[0], MY_IP[1],
                                                MY_IP[2], MY_IP[3]);
    show_update = __TRUE;
    return;
  }
  if (--dhcp_tout == 0) {
    /* A timeout, disable DHCP and use static IP address. */
    dhcp_disable ();
    sprintf((char *)lcd_text[1]," DHCP failed    " );
    show_update = __TRUE;
    dhcp_tout = 30 | 0x80000000;
    return;
  }
  if (dhcp_tout == 0x80000000) {
    dhcp_tout = 0;
    sprintf((char *)lcd_text[0]," IP address:");
    sprintf((char *)lcd_text[1]," %d.%d.%d.%d", MY_IP[0], MY_IP[1],
                                                MY_IP[2], MY_IP[3]);
    show_update = __TRUE;
  }
}

//
char* getSDData(){
	
	char str[500];
	FILE *f;
	int res;
	int i;
	char buf[16];
	
	 
	unsigned char buffer[1024]; 
	
	FINFO info;
	info.fileID = 0;
		
	//check if sd card is inserted
	res = finit(NULL);
	
	//
		if (res == 0) {
			sprintf(str,"<p>rSD/MMC Init ok</p>");
    }
		
	//
    if (res == 1) {
			sprintf(str, "<p>SD/MMC Init Failed\n\rInsert Memory card ...</p>");
			
    }else{
		//
			
				sprintf(str, "%s<p> Files existing on micro SD</p><ul>" , str);
				while (ffind ("M:*.*", &info) == 0)  { /* find whatever is in drive "M0:" */
				
				sprintf((char *)str,"%s <li> <b>%-32s</b> %5d bytes, ID: %04d</li>",
						str,
            info.name,
            info.size,
            info.fileID);
				
				}
				sprintf(str, "%s</ul>" , str);
				
				if (info.fileID == 0)  {
					sprintf((char *)str, "%s", "<p><br>No files...</b></p>");
					
				}
			
			
			//
			
		}	
		fclose(f);
		return str;
}
//
int main (void) {
  /* Main Thread of the TcpNet */

  init ();
	 SER_Init ();
  LEDrun = __TRUE;
  dhcp_tout = DHCP_TOUT;
  while (1) {
    timer_poll ();
    main_TcpNet ();
    dhcp_check ();
  }
}


void delay_ms (uint32_t Time)
{
    uint32_t i;
    
    i = 0;
    while (Time--) {
        for (i = 0; i < 16666; i++);
    }
}


