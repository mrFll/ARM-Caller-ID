#include "lpc17xx.h"
#include "i2c.h"

uint8_t buf[32];
extern void Delay(uint32_t delaydata);
                                                                        /* Defined for the I2C interrupt transmission of information  */
                                                                        /* Global Variables                     */
volatile uint8_t     I2C_sla;         /* From the I2C device address               */
volatile uint32_t    I2C_suba;        /* I2C device address within the sub-            */
volatile uint8_t     I2C_suba_num;    /* I2C subaddress bytes   */
volatile uint8_t     *I2C_buf;        /* Pointer to data buffer               */
volatile uint32_t    I2C_num;         /* To read / write the number of data       */
volatile uint8_t     I2C_end;         /* Marks the end of the I2C bus: the end of the bus is  */
                                                                        /* Set 1                          */
volatile uint8_t     I2C_suba_en; /* sub address control.
                                                0 - sub-sub-address or no address has been processed
                                                1 - read operation
                                                2-- write
                                        */
/* 
From device to device, the I2C communication protocol may vary, 
in the example below, the protocol uses repeated start to read data from or 
write to the device:
For master read: the sequence is: STA,Addr(W),offset,RE-STA,Addr(r),data...STO 
for master write: the sequence is: STA,Addr(W),length,RE-STA,Addr(w),data...STO
Thus, in state 8, the address is always WRITE. in state 10, the address could 
be READ or WRITE depending on the I2CCmd.
*/   

/*****************************************************************************
** Function name:		I2C0_IRQHandler
**
** Descriptions:		I2C0 interrupt handler, deal with master mode
**						only.
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void I2C0_IRQHandler(void)  
{
  uint8_t StatValue;
  StatValue = LPC_I2C0->I2STAT & 0xf8;
	
  switch ( StatValue )
  {
	case 0x08:		 										/* START condition sent */
	if (I2C_suba_en == 1)
	{	LPC_I2C0->I2DAT = I2C_sla &0xfe;	
	}
	else
	{	LPC_I2C0->I2DAT = I2C_sla;	
	}	
	LPC_I2C0->I2CONCLR = (1 << 3)|(1 << 5);
	break;
	
	case 0x10:												/*A repeated START condition */ 
	LPC_I2C0->I2DAT = I2C_sla;
	LPC_I2C0->I2CONCLR = 0x28;
	break;
	
	case 0x18:											/*  ACK has to send data*/
	case 0x28:											/* I2DAT data sent, ACK has */
	if (I2C_suba_en == 0)
	{
	  
	  if(I2C_num>0)
	  {  
		  LPC_I2C0->I2DAT = *I2C_buf++; 			
		  LPC_I2C0->I2CONCLR = 0x28;
		  I2C_num--;
		  Delay(1);
	  }
	  else
	  {
		LPC_I2C0->I2CONSET = (1 << 4); 				
		LPC_I2C0->I2CONCLR = 0x28;  				
		I2C_end =1;
	  }
		
	}
	if(I2C_suba_en == 1)
	{
		if(I2C_suba_num == 2)
		{
		  LPC_I2C0->I2DAT = ((I2C_suba >> 8) & 0xff);
		  LPC_I2C0->I2CONCLR = 0x28;
		  I2C_suba_num--; 
		  break;
		 }
		if(I2C_suba_num == 1)
		{
		  LPC_I2C0->I2DAT = (I2C_suba & 0xff);
		  LPC_I2C0->I2CONCLR = 0x28;
		  I2C_suba_num--; 
		  break;
		 }
		 if(I2C_suba_num == 0)
		 {
		  LPC_I2C0->I2CONCLR = 0x08;
		  LPC_I2C0->I2CONSET = 0x20;
		  I2C_suba_en = 0;
		  break;		  		 
		 }   	
	}

	if ( I2C_suba_en == 2 )
	{
	  if(I2C_suba_num > 0)
	  {
		  if(I2C_suba_num == 2)
		  {
			  LPC_I2C0->I2DAT = ((I2C_suba >> 8) & 0xff);
			  LPC_I2C0->I2CONCLR = 0x28;
			  I2C_suba_num--; 
			  break;
		  }
		  if(I2C_suba_num == 1)
		  {
			  LPC_I2C0->I2DAT = (I2C_suba & 0xff);
			  LPC_I2C0->I2CONCLR = 0x28;
			  I2C_suba_num--; 
			  I2C_suba_en  = 0;
			  break;
		  }
	  }
	}
	break;
	
	case 0x40:									 /* Sent SLA + R, ACK has */
	if (I2C_num <= 1)
	{	LPC_I2C0->I2CONCLR = 1 << 2;
	}
	else
	{
		LPC_I2C0->I2CONSET = 1 << 2;			/* assert ACK after data is received */
	 }
	LPC_I2C0->I2CONCLR = 0x28;
	break;

	case 0x20:								/* regardless, it's a NACK * / / * Sent SLA + W, has been receiving non-response */
	case 0x30:	 							/* I2DAT data sent, received non-response     */	
	case 0x38:		
	case 0x48:								/* Sent SLA + R, has been receiving non-response              */
	LPC_I2C0->I2CONCLR = 0x28;
	I2C_end = 0xFF;
	break;
		
	case 0x50:								/* Data byte has been received, ACK has been returned */
	*I2C_buf++ = LPC_I2C0->I2DAT;
	I2C_num--;
	if (I2C_num ==1)					    /* Receiving the last byte             */
	{	LPC_I2C0->I2CONCLR = 0x2c;	   		/* STA,SI,AA = 0                */
	}
	else
	{	LPC_I2C0->I2CONSET = 0x04;			
		LPC_I2C0->I2CONCLR = 0x28;	   
	}
	break;

	case 0x58: 								/* Received data byte, has returned to non-response */	
	*I2C_buf++ = LPC_I2C0->I2DAT;
	LPC_I2C0->I2CONSET = 0X10;				/* assert ACK after data is received */
	LPC_I2C0->I2CONCLR = 0X28;
	I2C_end = 1;
	break;

	default:
	break;
  }

}
/*****************************************************************************
** Function name:		I2CInit
**
** Descriptions:		Initialize I2C controller
**
** parameters:			I2c mode is either MASTER or SLAVE
** Returned value:		true or false, return false if the I2C
**				interrupt handler was not installed correctly
** 
*****************************************************************************/
uint32_t I2CInit( uint32_t I2cMode ) 
{
  LPC_SC->PCONP |= (1 << 19);

  /* set PIO0.27 and PIO0.28 to I2C0 SDA and SCK */
  /* function to 01 on both SDA and SCK. */
  LPC_PINCON->PINSEL1 &= ~0x03C00000;
  LPC_PINCON->PINSEL1 |= 0x01400000;	
 
  /*--- Reset registers ---*/
  LPC_I2C0->I2SCLL   = I2SCLL_SCLL;
  LPC_I2C0->I2SCLH   = I2SCLH_SCLH;
  if ( I2cMode == I2CSLAVE )
  {
	LPC_I2C0->I2ADR0 = 0xA0;
  }    

  /* Install interrupt handler */
  NVIC_EnableIRQ(I2C0_IRQn);

  LPC_I2C0->I2CONSET = I2CONSET_I2EN;
  return( 1 );
}

/*
*********************************************************************************************************
** Function name: I2C_WriteNByte ()
** Function: to have the child write N bytes of data the device address
** Entry parameter: sla device from the address
** Suba_type subaddress structure 1 - single-byte address 3-8 + X structure 2-- byte address
** Suba internal device physical address
** * S will be a pointer to data to be written
** Num the number of data to be written
** Output Parameters: 1 operation is successful
** 0 operation failed
*********************************************************************************************************
*/
uint8_t I2C_WriteNByte(uint8_t sla, uint8_t suba_type, uint32_t suba, uint8_t *s, uint32_t num)
{
    if (num > 0)                                                /* If you read the number is 0, return error  */
	{
		if (suba_type == 1)
		{
		    I2C_sla         = sla;                                      /* Read from the device address               */
		    I2C_suba        = suba;                                     /* Device subaddress                  */
		    I2C_suba_num    = 1;    /* 1 byte device subaddress			*/
	   	}
		if (suba_type == 2)
		{
		    I2C_sla         = sla;                                      /* Read from the device address               */
		    I2C_suba        = suba;                                     /* Device subaddress                   */
		    I2C_suba_num    = 2;                                        /* 1 byte device subaddress			*/
	   	}
		if (suba_type == 3)
		{
		    I2C_sla         = sla + ((suba >> 7 )& 0x0e);                                      /* Read from the device address               */
		    I2C_suba        = suba;                                     /* Device subaddress                   */
		    I2C_suba_num    = 1;                                        /* 1 byte device subaddress			*/
	   	}

	    I2C_buf     	= s;                                                /* Data                         */
	    I2C_num     	= num;                                              /* The number of data                     */
	    I2C_suba_en 	= 2;                                                /* A sub-address, write            */
	    I2C_end     	= 0;
  LPC_I2C0->I2CONCLR = (1 << 2)|
  					   (1 << 3)|
					   (1 << 5);  /* Clear SI flag */

  LPC_I2C0->I2CONSET = (1 << 5)|
  					   (1 << 6);	/* Set Start flag */

	while(I2C_end == 0);
		if(I2C_end)
		{
 			Delay(20);
			return 1;
		}
		else
		{
			Delay(20);
			return 0;
		}
  }
  Delay(20);
  return 0;
}
/*
*********************************************************************************************************
** Function name: I2C_ReadNByte ()
** Function: sub-address from a device to read N bytes starting at any address data
** Entry parameter: sla device from the address
** Suba_type subaddress structure 1 - single-byte address 2-8 + X structure 2-- byte address
** Suba device subaddress
** S data receive buffer pointer
** Num the number of read
** Output Parameters: 1 operation is successful
** 0 operation failed
*********************************************************************************************************
*/
uint8_t I2C_ReadNByte (uint8_t sla, uint8_t suba_type, uint32_t suba, uint8_t *s, uint32_t num)
{
	if (num > 0)
	{
		if (suba_type == 1)
		{
			I2C_sla         = sla + 1;                                  /* Read from the device address~{#,~}R=1          */
			I2C_suba        = suba;                                     /* Device subaddress                   */
			I2C_suba_num    = 1;                                        /*1 byte device subaddress            */
		}
		if (suba_type == 2)
		{
			I2C_sla         = sla + 1;                                  /* Read from the device address~{#,~}R=1          */
			I2C_suba        = suba;                                     /* Device subaddress                   */
			I2C_suba_num    = 2;                                        /* 1 byte device subaddress            */
		}
		if (suba_type == 3)
		{
			I2C_sla         = sla + ((suba >> 7 )& 0x0e) + 1;            /* Read from the device address~{#,~}R=1          */
			I2C_suba        = suba & 0x0ff;                              /* Device subaddress                   */
			I2C_suba_num    = 1;                                        /* 1 byte device subaddress            */
		}



	I2C_buf     	= s;                                                /* Data receive buffer pointer           */
	I2C_num     	= num;                                              /* To read the number of                 */
	I2C_suba_en	 	= 1;                                                /* A sub-address read                   */
	I2C_end     	= 0;
	
  LPC_I2C0->I2CONCLR = (1 << 2)|
  					   (1 << 3)|
					   (1 << 5);  /* Clear SI flag */

  LPC_I2C0->I2CONSET = (1 << 5)|
  					   (1 << 6);	/* Set Start flag */
	
	while(I2C_end == 0);
		if(I2C_end)
		{
 			Delay(20);
			return 1;
		}
		else
		{
			Delay(20);
			return 0;
		}

	}
	Delay(20);
	return 0;
}
/******************************************************************************
**                            End Of File
******************************************************************************/

