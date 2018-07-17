#include "Net_Config.h"
#include <stdio.h>
#include <string.h>



/* http_demo.c */
extern U16 AD_in (U32 ch);
extern U8  get_button (void);
extern char*  get_cal_status (void);
extern void  show_massage_to_display (char *data);
extern void finish_the_call(void);


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

extern BOOL LEDrun;
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
	
	P2 = 0;
  LEDrun = __TRUE;
	
  if (len == 0) {
    /* No data or all items (radio, checkbox) are off. */
    LED_out (P2);
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
        finish_the_call(); // diable the pin that controll the relay to hold the call -> call will finish
      }
      else if (str_scomp (var, "led0=on") == __TRUE) {
        P2 |= 0x01;
      }
      else if (str_scomp (var, "led1=on") == __TRUE) {
        P2 |= 0x02;
      }
      else if (str_scomp (var, "led2=on") == __TRUE) {
        P2 |= 0x04;
      }
      else if (str_scomp (var, "led3=on") == __TRUE) {
        P2 |= 0x08;
      }
      else if (str_scomp (var, "ctrl=Browser") == __TRUE) {
        LEDrun = __FALSE;
      }
    }
  }while (dat);
  free_mem ((OS_FRAME *)var);
  LED_out (P2);

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
		case 'h':
			len = sprintf((char *)buf,"%s", getSDData());
			break;
		//-------------------------------------------------------------------
		case 'y':
      /* Button state - xml file 'button.cgx' */
      len = sprintf((char *)buf,"<checkbox><id>button%c</id><on>%s</on></checkbox>",
                    env[1],(get_button () & (1 << (env[1]-'0'))) ? "true" : "false");
      break;
		//-------------------------------------------------------------------
		case 'r':
			
			len = sprintf((char *)buf,"<ring><on>%s</on></ring>",get_cal_status ());
			break;
			
	}
	return ((U16)len);
}

