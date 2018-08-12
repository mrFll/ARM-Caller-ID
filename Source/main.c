//struct __FILE { int handle; };

// frequence zone variables
	//volatile uint32_t inputFreq, inputFreq2;
	//char Variablestr [10]; 
	//char Variablestr2 [10];

//char p [10];

#include <stdio.h>
#include "RTL.h"
#include "lpc17xx.h"
#include "string.h"
#include "Net_Config.h"
#include "RTL.h"                      
#include "File_Config.h"
#include "Serial.h"
#include "display4bit.h"

// ring detector variables
#define CALL_PIN 0x4000000
#define DTMF_DATA_INTR_PIN 0x20000
#define DTMF_DATA_PINS 0x3C0000
#define CALL_FINISHER_NUMBER 0

#define NORMAL_MASSAGE "Central Ready"

// -------------------- call state variables ----------------------
enum state{
	NO_CALL = 1,
	RINGING,
	ANSWERED 
};
	#define NO_CALL 1
	#define RINGING 2
	#define ANSWERED 3
enum state callerIdStaus = NO_CALL;
// -----------------------------------------------------------------
//char ringMessage [20];

BOOL getTone = __FALSE; // USED
	
	// ring ditector variables
	uint8_t ring_count = 0;

	
	uint8_t NUMBER_OF_WAITING_RINGS = 3;
	
	
	
	// tele data variable
	uint8_t dtmfData = 0;
	char dtmfDataString [10];


BOOL show_update;
BOOL tick;
U32  dhcp_tout;


extern LOCALM localm[];                       /* Local Machine Settings      */
#define MY_IP localm[NETIF_ETH].IpAdr
#define DHCP_TOUT   50                        /* DHCP timeout 5 seconds      */
													

/************************************************************
*************************************************************
************************ Methods ****************************
*************************************************************
**************************************************************/
void show_massage_to_display(char *data);
void delay_main(uint32_t Time);
//
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
												Ethernet
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
static void dhcp_check () {
  

  if (tick == __FALSE || dhcp_tout == 0) {
    return;
  }
//  MY_IP[0] = 192; MY_IP[1] = 168 ; MY_IP[2] = 1; MY_IP[3] = 98;
  if (mem_test (&MY_IP, 0, IP_ADRLEN) == __FALSE && !(dhcp_tout & 0x80000000)) {
    /* Success, DHCP has already got the IP address. */
    dhcp_tout = 0;
    //sprintf((char *)lcd_text[0]," IP address:");
    //sprintf((char *)lcd_text[1]," %d.%d.%d.%d", MY_IP[0], MY_IP[1],MY_IP[2], MY_IP[3]);
    show_update = __TRUE;
    return;
  }
  if (--dhcp_tout == 0) {
    /* A timeout, disable DHCP and use static IP address. */
    dhcp_disable ();
    //sprintf((char *)lcd_text[1]," DHCP failed    " );
    show_update = __TRUE;
    dhcp_tout = 30 | 0x80000000;
    return;
  }
  if (dhcp_tout == 0x80000000) {
    dhcp_tout = 0;
    //sprintf((char *)lcd_text[0]," IP address:");
    //sprintf((char *)lcd_text[1]," %d.%d.%d.%d", MY_IP[0], MY_IP[1],MY_IP[2], MY_IP[3]);
    show_update = __TRUE;
  }
}




static void timer_poll () {
  /* System tick timer running in poll mode */

  if (SysTick->CTRL & 0x10000) {
    /* Timer tick every 100 ms */
    timer_tick ();
    tick = __TRUE;
  }
}
//
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
												TelePhone
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
void finish_the_call(){
	
	// clear relay pin
	LPC_GPIO0->FIOCLR = 0x2000000;
	
	// change call status
	callerIdStaus = NO_CALL;
}
void answer_the_call(){
	
	// set relay pin
	LPC_GPIO0->FIOSET = 0x2000000;
	
	// change call status
	callerIdStaus = ANSWERED;
}
char* get_State_Enum(void){
	
	switch(callerIdStaus){
		
		case NO_CALL:
			return "no call";
			break;
		
		case RINGING:
			return "ringing";
			break;
			
		case ANSWERED:
			return "answerd";
			break;
		
	}
	
}
void EINT3_IRQHandler(){
	// check if interrupt happened
	if((LPC_GPIOINT->IntStatus & 0x01) == 0x01)
	{	
		
		if((LPC_GPIOINT->IO0IntStatR & CALL_PIN) == CALL_PIN){
			
			LPC_GPIOINT->IO0IntClr = CALL_PIN;					// clear the interupt
			// state of incoming call
				if(ring_count == NUMBER_OF_WAITING_RINGS){		// after  waiting beeps
		
					answer_the_call();
					ring_count = 0;
					show_massage_to_display("call starts");
					delay_main(2000);
	
				}
				else if((ring_count < NUMBER_OF_WAITING_RINGS) & (callerIdStaus != ANSWERED)) // before  waiting beeps  
				{
					ring_count++;												
					callerIdStaus = RINGING;
					show_massage_to_display("      call      ");
					delay_main(5000);
					show_massage_to_display(NORMAL_MASSAGE);
				}
			
		}else if(((LPC_GPIOINT->IO0IntStatR & DTMF_DATA_INTR_PIN) == DTMF_DATA_INTR_PIN) && (callerIdStaus == ANSWERED)){
			// **************** DTMF tone detector zone ****************
			LPC_GPIOINT->IO0IntClr = DTMF_DATA_INTR_PIN;					// clear the interupt
			// state of incoming data from user
			
			// read pins to get the binary number
			dtmfData = ((LPC_GPIO0->FIOPIN & DTMF_DATA_PINS) >> 18);
			
			// Working with data that got from user 
			if(dtmfData == CALL_FINISHER_NUMBER){
				
				// user press the key that call wil finishing
				finish_the_call();		// diable the router that hold the call
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
void TIMER0_IRQHandler (void){
  LPC_TIM0->IR = 1;   
	getTone = __FALSE;
	//show_massage_to_display("get Tone Change");
	LPC_TIM0->TCR = 0;		   //disable counter
	
	show_massage_to_display("TIMER0_IRQH");
}
//
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
													Display
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
void show_massage_to_display(char *data){
	command(0x01);        //LCD CLEAR
	command(0x80);  //X_Y CONFIG  satr 1
	lcd_putsf(data);
}

//
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
													Main
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
static void init () {
  init_TcpNet ();

  /* Setup and enable the SysTick timer for 100ms. */
  SysTick->LOAD = (SystemCoreClock / 10) - 1;
  SysTick->CTRL = 0x05;
}

void timer0Inir(){
	// TODO check later
	LPC_TIM0->CTCR = 0;                              // set T/C for Timer
  LPC_TIM0->PR = 0;	                             // no prescale
  LPC_TIM0->MR0 = 25000000;
  LPC_TIM0->TCR = 2;	                             // reset counter , disable counter
  LPC_TIM0->MCR = 3;			                     //when T0MR0 is match with T0TC , this register generate interrupt and reset counter(T0TC)

  //NVIC_SetPriority(TIMER0_IRQn,0); /* Default priority group 0, can be 0(highest) - 31(lowest) */
  NVIC_EnableIRQ(TIMER0_IRQn); /* Enable Timer0 Interrupt */

}
void uartInit(){
	
	// FSK SYSTEM Uart Config
	LPC_SC->PCONP |=(1<<3);	//UART 0 POWER ON
	LPC_PINCON->PINSEL0 |= (1 << 6);	// Pin P0.3 used as RXD0
	
	/**
	*	8-bit character length
	*	1 stop bit
	*	Disable parity generation and checking 
	*	Disable break transmission
	*	Enable access to Divisor Latches
	*/
	LPC_UART0->LCR = 0x83; 
	
	// DOUT baud data -> 1200 
	LPC_UART0->DLM =((25000000/16)/1200)/ 256;							
  LPC_UART0->DLL = ((25000000/16)/1200)% 256;	
	
	LPC_UART0->LCR = 0x03;
	LPC_UART0->FCR = 0x03;	// Enable and reset RX FIFO
	LPC_UART0->IER = 0x01;	// Enables the Receive Data Available interrupt 
	
	NVIC_SetPriority(UART0_IRQn,0);	// Default priority group 0, can be 0(highest) - 31(lowest)
  NVIC_EnableIRQ(UART0_IRQn);	// Enable UART0 Interrupt	
}
void gpio_interrupt_set(){
	
	LPC_PINCON->PINSEL1 = 0x0;						// set pins to GPIO
	LPC_GPIO0->FIODIR = 0x2000000;				// make pins input and pin 25 output
	LPC_GPIO0->FIOCLR = 0x2000000;
	
	// TODO check near to each other and check with new chema
	LPC_GPIOINT->IO0IntClr = (CALL_PIN | DTMF_DATA_INTR_PIN);		
	LPC_GPIOINT->IO0IntEnR = (CALL_PIN | DTMF_DATA_INTR_PIN);
	
	NVIC_EnableIRQ(EINT3_IRQn);	
	
}
void Enable_call_timer_main(void){
	
	LPC_SC->PCONP |= 1 << 22;         //timer2 power on
	
	LPC_SC->PCLKSEL0 |= 3 << 2;			 // Clock for timer = CCLK/8
	LPC_TIM2->MR0 = 12599999;       //1sec
	LPC_TIM2->MCR = 3;              //interrupt and reset control
                                   //3 = Interrupt & reset timer2 on match
	NVIC_EnableIRQ(TIMER2_IRQn);    //enable timer2 interrupt
	//LPC_TIM2->TCR = 0x1;              //enable Timer2
}
void delay_main(uint32_t Time){
	uint32_t i;
	i = 0;
	while(Time--)
	{
		for(i=0; i<5000; i++);
	}
}
void delay_ms (uint32_t Time){
    uint32_t i;
    i = 0;
    while (Time--) {
        for (i = 0; i < 16666; i++);
    }
}






int main (void) {
  

  init ();
	SER_Init ();	
	lcd_init_4bit();
	
	gpio_interrupt_set();	
	
	uartInit();	
	
	dhcp_tout = DHCP_TOUT;
	
	show_massage_to_display("Ready master :)");

	timer0Inir();
	
  while (1) {
    timer_poll ();
    main_TcpNet ();
    dhcp_check ();
  }
}




