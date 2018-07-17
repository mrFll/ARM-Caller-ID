
#include <stdio.h>
#include "RTL.h"
#include "lpc17xx.h"
#include "string.h"

#include "Net_Config.h"

#include "RTL.h"                      
#include "File_Config.h"
#include "Serial.h"

#include "display4bit.h"

struct __FILE { int handle; };
/*
FILE __stdout;
FILE __stdin;
*/

// ring detector variables
#define CALL_PIN 0x4000000
#define DTMF_DATA_INTR_PIN 0x20000
#define DTMF_DATA_PINS 0x3C0000
#define CALL_FINISHER_NUMBER 0

#define NORMAL_MASSAGE "Central Ready"


char ringMessage [20];

// frequence zone variables
	volatile uint32_t inputFreq, inputFreq2;
	char Variablestr [10]; 
	char Variablestr2 [10];
	
	// ring ditector variables
	uint8_t ring_count = 0;
	
	uint8_t call_situaions = 1;
	#define NO_CALL 1
	#define RINGING 2
	#define ANSWERED 3
	
	uint8_t NUMBER_OF_WAITING_RINGS = 3;
	
	// call talking variables
	//uint16_t call_time_duration_Sec = 5;
	//uint16_t duration_caounter = 0;
	
	char p [10];
	//char telephone_number [30];
	
	// tele data variable
	uint8_t dtmfData = 0;
	char dtmfDataString [10];


// led variables
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

/************************************************************
													Methods
**************************************************************/
void show_massage_to_display(char *data){
	
	command(0x01);        //LCD CLEAR
	command(0x80);  //X_Y CONFIG  satr 1
	lcd_putsf(data);
}
//
void delay_main(uint32_t Time){
	uint32_t i;
	i = 0;
	while(Time--)
	{
		for(i=0; i<5000; i++);
	}
}
//
void delay_ms (uint32_t Time){
    uint32_t i;
    i = 0;
    while (Time--) {
        for (i = 0; i < 16666; i++);
    }
}

//
static void init () {
  /* Add System initialisation code here */ 

  init_io ();
  init_TcpNet ();

  /* Setup and enable the SysTick timer for 100ms. */
  SysTick->LOAD = (SystemCoreClock / 10) - 1;
  SysTick->CTRL = 0x05;
}

//
void finish_the_call(){
	LPC_GPIO0->FIOCLR = 0x2000000;
}
//
static void timer_poll () {
  /* System tick timer running in poll mode */

  if (SysTick->CTRL & 0x10000) {
    /* Timer tick every 100 ms */
    timer_tick ();
    tick = __TRUE;
  }
}

//
char* get_cal_status(void){
	
	switch (call_situaions){
		
		case NO_CALL:
			
			return "NO CALL";
			break;
		
		case RINGING:
			return "ringing";
			break;
		
		case ANSWERED:
			
			return "call starts";
			break;
		
		default:
			
			return "no status";
			break;		
	}
	
}
//
static void init_io () {

 /* Configure the GPIO for LEDs. */
  //LPC_GPIO1->FIODIR =  LED1 | LED2 |LED3 | LED4;           /* LEDs PORT1 25...28 are Output */
  //LPC_GPIO1->FIOCLR =  LED1 | LED2 | LED3 | LED4;		       /* turn off all the LEDs */

  /* Configure the GPIO for Push Buttons */
 // LPC_PINCON->PINSEL4 &= ~((3<<20)|(3<<22)|(3<<24));
 // LPC_GPIO2->FIODIR   &= ~((1<<10)|(1<<11)|(3<<12));
  
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
	
	 U32 val = 0;

  if ((LPC_GPIO2->FIOPIN & (1 << 10)) == 0) {
    /* INT0 button */
    val |= 0x01;
  }
  if ((LPC_GPIO2->FIOPIN & (1 << 11)) == 0) {
    /* Key1 */
    val |= 0x02;
  }
  if ((LPC_GPIO2->FIOPIN & (1 << 12)) == 0) {
    /* Key2 */
    val |= 0x04;
  }
  return (val);
	
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
void gpio_interrupt_set(){
	
	LPC_PINCON->PINSEL1 = 0x0;						// set pins to GPIO
	LPC_GPIO0->FIODIR = 0x2000000;				// make pins input and pin 25 output
	LPC_GPIO0->FIOCLR = 0x2000000;
	
	LPC_GPIOINT->IO0IntClr = (CALL_PIN | DTMF_DATA_INTR_PIN);		
	LPC_GPIOINT->IO0IntEnR = (CALL_PIN | DTMF_DATA_INTR_PIN);
	
	//LPC_GPIOINT->IO0IntClr = DTMF_DATA_INTR_PIN;		
	//LPC_GPIOINT->IO0IntEnR = DTMF_DATA_INTR_PIN;
	
	NVIC_EnableIRQ(EINT3_IRQn);	
	
}
//
void EINT3_IRQHandler(){
	// check if interrupt happened
	if((LPC_GPIOINT->IntStatus & 0x01) == 0x01)
	{	
		
		if((LPC_GPIOINT->IO0IntStatR & CALL_PIN) == CALL_PIN){
			
			LPC_GPIOINT->IO0IntClr = CALL_PIN;					// clear the interupt
			// state of incoming call
				if(ring_count == NUMBER_OF_WAITING_RINGS){		// after  waiting beeps
		
					call_situaions = ANSWERED;													// the call is answered
					ring_count = 0;
					show_massage_to_display("call starts");
					delay_main(2000);		
					LPC_GPIO0->FIOSET = 0x2000000;							// send signal to digital center that we accept the call - relay ON
	
				}
				else if((ring_count < NUMBER_OF_WAITING_RINGS) & (call_situaions != ANSWERED)) // before  waiting beeps
				{
					show_massage_to_display("      call      ");
					ring_count++;												
					call_situaions = RINGING;
					delay_main(5000);
					show_massage_to_display(NORMAL_MASSAGE);
				}
			
		}else if(((LPC_GPIOINT->IO0IntStatR & DTMF_DATA_INTR_PIN) == DTMF_DATA_INTR_PIN) && (call_situaions == ANSWERED)){
			
			// TODO remove it later
			show_massage_to_display(":)");
			delay_main(2000);
			show_massage_to_display(NORMAL_MASSAGE);
			
			
			LPC_GPIOINT->IO0IntClr = DTMF_DATA_INTR_PIN;					// clear the interupt
			// state of incoming data from user
			
			// read pins to get the binary number
			dtmfData = ((LPC_GPIO0->FIOPIN & DTMF_DATA_PINS) >> 18);
			
			// Working with data that got from user 
			if(dtmfData == CALL_FINISHER_NUMBER){
				
				// user press the key that call wil finishing
				finish_the_call();		// diable the router that hold the call
				call_situaions = NO_CALL;
				show_massage_to_display("Call ended");
				delay_main(5000);
				show_massage_to_display(NORMAL_MASSAGE);
				
			}else{
				// make string from input variable
			sprintf(dtmfDataString, "Code: %d" , dtmfData);
			
			// show the data
			show_massage_to_display(dtmfDataString);
			delay_main(5000);
			show_massage_to_display(NORMAL_MASSAGE);
			}
		}
	}
}
//
void Enable_call_timer_main(void){
	
	LPC_SC->PCONP |= 1 << 22;         //timer2 power on
	
	LPC_SC->PCLKSEL0 |= 3 << 2;			 // Clock for timer = CCLK/8
	LPC_TIM2->MR0 = 12599999;       //1sec
	LPC_TIM2->MCR = 3;              //interrupt and reset control
                                   //3 = Interrupt & reset timer2 on match
	NVIC_EnableIRQ(TIMER2_IRQn);    //enable timer2 interrupt
	//LPC_TIM2->TCR = 0x1;              //enable Timer2
}
// this method show welcoming message on LCD
void welcoming(){
	show_massage_to_display("Welcome to:");
	delay_main(10000);
	show_massage_to_display("Parham Fallah's");
	delay_main(10000);
	show_massage_to_display("Final Project");
	delay_main(10000);
	show_massage_to_display("BIHE Fall 2016");
	delay_main(10000);
	show_massage_to_display(NORMAL_MASSAGE);
}


//
int main (void) {
  /* Main Thread of the TcpNet */

  init ();
	SER_Init ();
		
	lcd_init_4bit();
	
	show_massage_to_display("BOOT ...");
	
	gpio_interrupt_set();	
	
	LEDrun = __TRUE;
  dhcp_tout = DHCP_TOUT;
	
	welcoming();
	
  while (1) {
    timer_poll ();
    main_TcpNet ();
    dhcp_check ();
  }
}

