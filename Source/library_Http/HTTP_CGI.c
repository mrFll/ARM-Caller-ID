#include "Net_Config.h"
#include <stdio.h>
#include <string.h>
#include "lpc17xx.h"

/* http_demo.c */
extern U16 AD_in (U32 ch);
extern U8  get_button (void);
extern char*  get_State_Enum (void);
extern void  show_massage_to_display (char *data);
extern void finish_the_call(void);
extern void answer_the_call(void);
extern void invertRelay(uint8_t relayNumb);
extern void delay_main(uint32_t Time)


/* at_System.c */
extern  LOCALM localm[];
#define LocM   localm[NETIF_ETH]

/* Net_Config.c */
extern struct tcp_cfg   tcp_config;
extern struct http_cfg  http_config;
#define tcp_NumSocks    tcp_config.NumSocks
#define tcp_socket      tcp_config.Scb
#define http_EnAuth     http_config.EnAuth
#define http_auth_passw http_config.Passw

extern void LED_out (U32 val);
extern char* getSDData ();
extern BOOL show_update;
extern U8   lcd_text[2][16+1];

/* Local variables. */
static U8 P2;
static char const state[][9] = {
  "FREE",
  "CLOSED",
  "LISTEN",
  "SYN_REC",
  "SYN_SENT",
  "FINW1",
  "FINW2",
  "CLOSING",
  "LAST_ACK",
  "TWAIT",
  "CONNECT"};

typedef struct {
  U16 xcnt;
  U16 unused;
} MY_BUF;
#define MYBUF(p)        ((MY_BUF *)p)



void cgi_process_var (U8 *qs) {
 
  U8 *var;
  int s[4];

  var = (U8 *)alloc_mem (40);
  do {
    /* Loop through all the parameters. */
    qs = http_get_var (qs, var, 40);
    /* Check the returned string, 'qs' now points to the next. */
    if (var[0] != 0) {
      /* Returned string is non 0-length. */
      if (str_scomp (var, "ip=") == __TRUE) {
        /* My IP address parameter. */
        sscanf ((const char *)&var[3], "%d.%d.%d.%d",&s[0],&s[1],&s[2],&s[3]);
        LocM.IpAdr[0]   = s[0];
        LocM.IpAdr[1]   = s[1];
        LocM.IpAdr[2]   = s[2];
        LocM.IpAdr[3]   = s[3];
      }
      else if (str_scomp (var, "msk=") == __TRUE) {
        /* Net mask parameter. */
        sscanf ((const char *)&var[4], "%d.%d.%d.%d",&s[0],&s[1],&s[2],&s[3]);
        LocM.NetMask[0] = s[0];
        LocM.NetMask[1] = s[1];
        LocM.NetMask[2] = s[2];
        LocM.NetMask[3] = s[3];
      }
      else if (str_scomp (var, "gw=") == __TRUE) {
        /* Default gateway parameter. */
        sscanf ((const char *)&var[3], "%d.%d.%d.%d",&s[0],&s[1],&s[2],&s[3]);
        LocM.DefGW[0]   = s[0];
        LocM.DefGW[1]   = s[1];
        LocM.DefGW[2]   = s[2];
        LocM.DefGW[3]   = s[3];
      }
      else if (str_scomp (var, "pdns=") == __TRUE) {
        /* Default gateway parameter. */
        sscanf ((const char *)&var[5], "%d.%d.%d.%d",&s[0],&s[1],&s[2],&s[3]);
        LocM.PriDNS[0]  = s[0];
        LocM.PriDNS[1]  = s[1];
        LocM.PriDNS[2]  = s[2];
        LocM.PriDNS[3]  = s[3];
      }
      else if (str_scomp (var, "sdns=") == __TRUE) {
        /* Default gateway parameter. */
        sscanf ((const char *)&var[5], "%d.%d.%d.%d",&s[0],&s[1],&s[2],&s[3]);
        LocM.SecDNS[0]  = s[0];
        LocM.SecDNS[1]  = s[1];
        LocM.SecDNS[2]  = s[2];
        LocM.SecDNS[3]  = s[3];
      }
    }
  }while (qs);
  free_mem ((OS_FRAME *)var);
}


/*--------------------------- cgi_process_data ------------------------------*/

void cgi_process_data (U8 code, U8 *dat, U16 len) {
	
	U8 passw[12],retyped[12];
  U8 *var,stpassw;
	
	switch (code) {
    case 0:
      /* Url encoded form data received. */
      break;

    default:
      /* Ignore all other codes. */
      return;
  }
	
  if (len == 0) {
    /* No data or all items (radio, checkbox) are off. */
    return;
  }
  stpassw = 0;
  var = (U8 *)alloc_mem (40);
  do {
    /* Parse all returned parameters. */
    dat = http_get_var (dat, var, 40);
    if (var[0] != 0) {
      /* Parameter found, returned string is non 0-length. */
			
			if (str_scomp (var, "canclebtn") == __TRUE) {
				// if user select finish button on the web, call page
        finish_the_call(); // diable the pin that controll the relay to hold the call -> call will finish
      }
			else if(str_scomp(var, "answ") == __TRUE){
				// if user select answer button on the web, call page
				answer_the_call();
			}
			else if(str_scomp(var, "setrel1") == __TRUE){
				invertRelay(1);
			}
			else if(str_scomp(var, "setrel2") == __TRUE){
				invertRelay(2);
			}
			else if(str_scomp(var, "setrel3") == __TRUE){
				invertRelay(3);
			}
			else if(str_scomp(var, "setrel4") == __TRUE){
				invertRelay(4);
			}
			else if(str_scomp(var, "rsub1") == __TRUE){
				show_massage_to_display(var);
			}
    }
  }while (dat);
  free_mem ((OS_FRAME *)var);

  if (stpassw == 0x03) {
    len = strlen ((const char *)passw);
    if (mem_comp (passw, retyped, len) == __TRUE) {
      /* OK, both entered passwords the same, change it. */
      str_copy (http_auth_passw, passw);
    }
  }
	
}


/*--------------------------- cgi_func --------------------------------------*/

U16 cgi_func (U8 *env, U8 *buf, U16 buflen, U32 *pcgi) {
	
  U32 len = 0;
  U8 id;
	
	switch(env[0]){
		
		case 'l':
      id = 1 << (env[2] - '0');
      len = sprintf((char *)buf,(const char *)&env[4],(P2 & id) ? "checked" : "");
			break;
		//-------------------------------------------------------------------
		case 'r':
			
			len = sprintf((char *)buf,"<ring><on>%s</on></ring>",get_State_Enum());
			break;
			
	}
	return ((U16)len);
}

