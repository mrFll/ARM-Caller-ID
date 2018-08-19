#include <stdio.h>
#include "RTL.h"
#include "lpc17xx.h"
#include "string.h"
#include "Net_Config.h"
#include "RTL.h"                      
#include "File_Config.h"
#include "Serial.h"
#include "display4bit.h"
#include "i2c.h"

// ring detector variables
#define CALL_PIN 0x4000000
#define DTMF_DATA_INTR_PIN 0x20000
#define DTMF_DATA_PINS 0x3C0000


//-------------------------------------------------- Display variables --------------------------------------------------

char realyCodes [4][5] = {{'*','1','1','1','#'},{'*','2','2','2','#'},{'*','3','3','3','#'},{'*','4','4','4','#'}};

uint8_t bs = 0;
#define BS_INDEX = 0x80;	// LCD 16X2 index
							/**
							*		0 -> normanl 	-> -  (default)
							*		1 -> ring 		-> R
							*		2 -> online 	-> O
							*/
							
char callerNumber [11] = {'_', '_', '_', '_', 'n', 'u', 'm', '_', '_', '_', '_'};
#define CALLER_NUMBER = 0x85;	// LCD 16X2 index							

char code [5] = {'C','o','d','e',':'};
uint8_t codeIndex = 0;
#define CODE = 0xCB;	// LCD 16X2 index	

BOOL relays [4] = {__FALSE, __FALSE, __FALSE, __FALSE};
#define RELAYS = 0xC6;	// LCD 16X2 index	



// ------------------------------------------------------------------------------------------------------------------------

BOOL getTone = __FALSE; 
	
// ring ditector variables
uint8_t ring_count = 0;
	
uint8_t NUMBER_OF_WAITING_RINGS = 4;
		
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
void  invertRelay(uint8_t relayNumb);
void showStatusOnDisplay();
void showCallerNumberOnDisplay();
void showToneCodeOnDisplay();
void showRelaysStatusOnDisplay();
void updateDisplay();
//
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
												EEPROM
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
void set_realay_code(char code[5] ,uint8_t relayNumb){
	I2C_WriteNByte( 0xa0, 1 ,(((relayNumb-1)*5)+1) ,code ,5);
}
char * get_relay_codes(uint8_t relayNumb){
	
	char result [5];
		I2C_ReadNByte (0xa0, 1, (((relayNumb-1)*5)+1), result, 5);		
	return result;
}

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
char show_number_in_lcd(uint8_t number){
	char cr[2];
	
	if(number != 0){
		
		switch(number){
			
			case 10:
				return '0';
				break;
			
			case 11:
				return '*';
				break;
			
			case 12:
				return '#';
				break;
			
			default:
				sprintf(cr,"%d", dtmfData);
				return cr[0];
				break;	
		}
	}
}
void finish_the_call(){
	
	// clear relay pin
	LPC_GPIO0->FIOCLR = 0x2000000;
	
	// change call status
	bs = 0;
	showStatusOnDisplay();
	
}
void answer_the_call(){
	
	// set relay pin
	LPC_GPIO0->FIOSET = 0x2000000;
	
	// change call status
	bs = 2;
	showStatusOnDisplay();
}
void EINT3_IRQHandler(){
	
	if((LPC_GPIOINT->IntStatus & 0x01) == 0x01)
	{			
		if((LPC_GPIOINT->IO0IntStatF & CALL_PIN) == CALL_PIN){
			
			LPC_GPIOINT->IO0IntClr = CALL_PIN;					// clear the interupt
			// state of incoming call
				if(ring_count == NUMBER_OF_WAITING_RINGS){		// after  waiting beeps

					answer_the_call();						// active the relay that start the call
					ring_count = 0;								// reset the number of 
					bs = 2;												// means that is phone is picked up and user can enter the code
					showStatusOnDisplay();	// show changed status on page
	
				}
				else if((ring_count < NUMBER_OF_WAITING_RINGS) & (bs != 2)) // before  waiting beeps  
				{
					ring_count++;									// increase ring count 
					bs = 1;												// change the state to ringing
					showStatusOnDisplay();	// show state on display
				}
			
		}else if(((LPC_GPIOINT->IO0IntStatR & DTMF_DATA_INTR_PIN) == DTMF_DATA_INTR_PIN)){
			
			// TODO TODO TODO TODO && (bs == 2)
			
			// **************** DTMF tone detector zone ****************
			LPC_GPIOINT->IO0IntClr = DTMF_DATA_INTR_PIN;					// clear the interupt
			// state of incoming data from user
			
			// read pins to get the binary number
			dtmfData = ((LPC_GPIO0->FIOPIN & DTMF_DATA_PINS) >> 18);
			
			updateDisplay();
			
			if(codeIndex < 4){				// get next char of code
				
				// claer lcd for code
				if(codeIndex < 1){
					code[0] = '_';
					code[1] = '_';
					code[2] = '_';
					code[3] = '_';
					code[4] = '_';
				}
				// init
				code[codeIndex] = show_number_in_lcd(dtmfData);
				
				// increase index of the code
				codeIndex++;
				
				showToneCodeOnDisplay();
				
			}else if(codeIndex == 4){		// apply the code 

				uint8_t ic = 0;
				uint8_t jc = 0;
				BOOL validCode = __FALSE;
				BOOL checkNext = __TRUE;
				
				// init the last char of user input
				code[codeIndex] = show_number_in_lcd(dtmfData);
				
				// check all the validation list of codes
				for(ic = 0; ic < 4; ic++){
					// check all of code char to be valid
					for(jc = 0; jc < 5; jc++){
						checkNext &= (realyCodes[ic][jc] == code[jc]);
					}
					
					// action by the validation result
					if(checkNext){
						//  if user enter the valid code
							validCode = __TRUE;
						
							// invert relay
							invertRelay(ic+1);
						
							break;
						
					}else{
						checkNext = __TRUE;
					}
					
				}
				
					if(validCode){
						show_massage_to_display("code applied");
					}else{
						show_massage_to_display("invalid code");
						
					}
					delay_main(2000);
					
					codeIndex = 0;
		
					code[0] = 'C';
					code[1] = 'o';
					code[2] = 'd';
					code[3] = 'e';
					code[4] = ':';
		
					updateDisplay();
				}
				
				
			}else if(codeIndex > 4){ 		// clean the code -- this status may not happend but prevention is better than cure
				
				// clear code on display
				codeIndex = 0;
				
				code[0] = 'C';
				code[1] = 'o';
				code[2] = 'd';
				code[3] = 'e';
				code[4] = ':';
				
				showToneCodeOnDisplay();
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
void UART0_IRQHandler(void) {
	
	show_massage_to_display("happy");
	delay_main(1000);
	
	/*
	char* tem ;
	sprintf(tem, "uart int %d",  UART0_cunter);
	show_massage_to_display(tem);
	delay_main(1000);
	
	if(UART0_cunter < 19){
		UART0_buffer[UART0_cunter]= LPC_UART0->RBR;
		UART0_cunter += 1;
	}else{
		LPC_UART0->IER = 0x00;	// Enables the Receive Data Available interrupt 
		show_massage_to_display(UART0_buffer);
		delay_main(10000);
	}
	*/
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
void updateDisplay(){
	
	show_massage_to_display(" ");
	
	// show board state
	showStatusOnDisplay();
	
	// show calling number 
	showCallerNumberOnDisplay();
	
	// show code 
	showToneCodeOnDisplay();
	
	// show relay status
	showRelaysStatusOnDisplay();
	
}
void showToneCodeOnDisplay(){
	uint8_t counter = 0;
	
	for(counter = 0; counter < 5; counter++){
		command(0xCB + counter);
		lcd_putchar(code[counter]);
	}
}
void showRelaysStatusOnDisplay(){
	uint8_t counter = 0;

	for(counter = 0; counter < 4; counter++){
		command(0xC6 + counter);
		
		// relay is open
		if(relays[counter] == __TRUE){
			lcd_putchar('^');
			
		// relay is close
		}else if(relays[counter] == __FALSE){
			lcd_putchar('_');
			
		// error
		}else{
			lcd_putchar('E');
		}
		
	}
}
void showCallerNumberOnDisplay(){
	uint8_t counter = 0;
	for(counter = 0; counter < 11; counter++){
		command(0x85 + counter);
		lcd_putchar(callerNumber[counter]);
	}
}
void showStatusOnDisplay(){
	
	command(0x80);
	
	switch(bs){
	
		case 0:
			lcd_putchar('-');			// normanl
			break;
		
		case 1:
			lcd_putchar('R');			// ring
			break;
		
		case 2:
			lcd_putchar('O');			// online
			break;
		
		default:
			lcd_putchar('E');			// error
			break;	
	}
}

//
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
													Controller
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
void changeRelaysStateOnBoard(uint8_t relayNumb){
	
		if(relays[relayNumb] == __TRUE){
			
			LPC_GPIO0->FIOSET = (1<<(5+relayNumb));
			
		}else if(relays[relayNumb] == __FALSE){
			
			LPC_GPIO0->FIOCLR = (1<<(5+relayNumb));
		}
}
void invertRelay(uint8_t relayNumb){
	if(relayNumb > 4 || relayNumb < 1){
		// check if input number is between 1 - 4
		show_massage_to_display("invalid relay number");
		delay_main(3000);
		updateDisplay();
	}else{
		// invert ralay variable status
		relays[relayNumb-1] = !relays[relayNumb-1];
		showRelaysStatusOnDisplay();
		changeRelaysStateOnBoard(relayNumb-1);
	}
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
	
	// set GPIO function for p0.5 p0.6 p0.7 p0.8
	LPC_PINCON->PINSEL0  &= ~ ((1<<10) | (1<<11) | (1<<12) | (1<<13) | (1<<14) | (1<<15) | (1<<16) | (1<<17));
	
	// set GPIO function for p0.16 to p0.31
	LPC_PINCON->PINSEL1 = 0x0;						// set pins to GPIO
	
	LPC_GPIO0->FIODIR = 0x20001E0;				// make pins input and pin p0.25 output
	LPC_GPIO0->FIOCLR = 0x20001E0;				// Clear this GPIO pins
	
	
	I2CInit( (uint32_t)I2CMASTER );		   // initialize I2c
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

	// Clear interupt for pins	
	LPC_GPIOINT->IO0IntClr = (CALL_PIN | DTMF_DATA_INTR_PIN);		
	
	// Rising interupt
	LPC_GPIOINT->IO0IntEnR = DTMF_DATA_INTR_PIN;
	
	// Falling interupt
	LPC_GPIOINT->IO0IntEnF = CALL_PIN;	
	
	// set function to handle events when interupt 
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
void Delay (uint32_t Time){
    uint32_t i;
    i = 0;
    while (Time--) {
        for (i = 0; i < 16666; i++);
    }
}
int main (void) {
  
  init ();
	//SER_Init ();	
	lcd_init_4bit();
	
	gpio_interrupt_set();	
	
	uartInit();	
	
	dhcp_tout = DHCP_TOUT;
	
	// show System info on desplay
	updateDisplay();

	timer0Inir();
		
  while (1) {
    timer_poll ();
    main_TcpNet ();
    dhcp_check ();
  }
}




