/*
 * TestTasks.c
 *
 * Created: 04.02.2021 18:54:30
 *  Author: KennisTower
 */ 





#include "TestTasks.h"

#include <asf.h>

#include "FreeRTOS.h"
#include "task.h"

#include "stdio.h"

#include "delay.h"
#include "port.h"
#include "spi.h"
#include "usart.h"

#include "conf_spi.h"


#include "wizchip_conf.h"
#include "ioLibrary/Internet/DNS/dns.h"
#include "socket.h"

#include "Application/Debug/debug.h"

#define PORT_WEBSERVER  80

//const char initData = "<!DOCTYPE html>		<html>		<body>		<h1>This is heading 1</h1>		<h2>This is heading 2</h2>		<h3>This is heading 3</h3>		<h4>This is heading 4</h4>		<h5>This is heading 5</h5>		<h6>This is heading 6</h6>		</body>		</html>";
uint8_t send_dat[1024]={0,};


wiz_NetInfo gWIZNETINFO = { .mac = {0x00,0x08,0xdc,0x78,0x91,0x71},
.ip = {192,168,1,10},
.sn = {255, 255, 255, 0},
.gw = {192, 168, 178, 1},
.dns = {192, 168, 178, 1},
.dhcp = NETINFO_STATIC};

#define ETH_MAX_BUF_SIZE	2048

unsigned char ethBuf0[ETH_MAX_BUF_SIZE];
unsigned char ethBuf1[ETH_MAX_BUF_SIZE];
//unsigned char ethBuf2[ETH_MAX_BUF_SIZE];
//unsigned char ethBuf3[ETH_MAX_BUF_SIZE];

uint8_t bLoopback = 1;
uint8_t bRandomPacket = 0;
uint8_t bAnyPacket = 0;
uint16_t pack_size = 0;

const uint8_t URL[] = "test.mosquitto.org";
uint8_t dns_server_ip[4] = {168,126,63,1};

void print_network_information(void);
void MQTT_operation(void);

uint8_t spiReadByte(void);
void spiWriteByte(uint8_t byte);
void csEnable(void);
void csDisable(void);
void W5x00Initialze(void);




#define TEST_TASK_PRIORITY      (tskIDLE_PRIORITY + 1)

//! [setup]
//! [buf_length]
#define BUF_LENGTH 20
//! [buf_length]
//! [slave_select_pin]
#define SLAVE_SELECT_PIN PIN_PA07
//! [slave_select_pin]
//! [buffer]
static uint8_t buffer[BUF_LENGTH] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13
};
//! [buffer]

//! [dev_inst]
struct spi_module spi_master_instance;
//! [dev_inst]
//! [slave_dev_inst]
struct spi_slave_inst slave;
//! [slave_dev_inst]
//! [setup]

//! [configure_spi]
void configure_spi_master(void)
{
	//! [config]
	struct spi_config config_spi_master;
	//! [config]
	//! [slave_config]
	struct spi_slave_inst_config slave_dev_config;
	//! [slave_config]
	/* Configure and initialize software device instance of peripheral slave */
	//! [slave_conf_defaults]
	spi_slave_inst_get_config_defaults(&slave_dev_config);
	//! [slave_conf_defaults]
	//! [ss_pin]
	slave_dev_config.ss_pin = SLAVE_SELECT_PIN;
	//! [ss_pin]
	//! [slave_init]
	spi_attach_slave(&slave, &slave_dev_config);
	//! [slave_init]
	/* Configure, initialize and enable SERCOM SPI module */
	//! [conf_defaults]
	spi_get_config_defaults(&config_spi_master);
	//! [conf_defaults]
	//! [mux_setting]
	config_spi_master.mux_setting = SPI_SIGNAL_MUX_SETTING_F;
	//! [mux_setting]
	config_spi_master.pinmux_pad0 = PINMUX_PA09C_SERCOM0_PAD1;
	config_spi_master.pinmux_pad1 = PINMUX_PA10C_SERCOM0_PAD2;
	config_spi_master.pinmux_pad2 = PINMUX_PA11C_SERCOM0_PAD3;
	config_spi_master.pinmux_pad3 = PINMUX_UNUSED;

	//! [init]
	spi_init(&spi_master_instance, SERCOM0, &config_spi_master);
	//! [init]

	//! [enable]
	spi_enable(&spi_master_instance);
	//! [enable]
	
	debugMSG("SPI Initiated");

}
//! [configure_spi]





uint16_t calcu_len(void)
{
	return strlen(send_dat);
}

void sendHeader(uint8_t sn)
{
	uint8_t dat[128]={0,};
	uint8_t dat_t[128]={0,};
	uint8_t *dat_temp;
	dat_temp = "HTTP/1.1 200 OK\r\n";                       strcat(dat, dat_temp);
	dat_temp = "Content-Type: text/html\r\n";                   strcat(dat, dat_temp);
	dat_temp = "Connection: close\r\n";                     strcat(dat, dat_temp);
	sprintf(dat, "%sContent-Length: %d", dat, calcu_len());
	dat_temp = "\r\n\r\n";                              strcat(dat, dat_temp);
	send(sn, (uint8_t *)dat, strlen(dat));
	debugMSG("%s", dat);
}
void sendData(uint8_t sn)
{
	uint16_t i;
	send(sn, (uint8_t *)send_dat, strlen(send_dat));
	debugMSG("%s", send_dat);
	for(i=0;i<1024;i++){
		send_dat[i]=0;
	}
}

char proc_http(uint8_t sn, char * buf)
{
    if((buf[0]=='G')&&(buf[1]=='E')&&(buf[2]=='T')&&(buf[3]==' ')){ // GET_Request
        if((buf[5]=='?')){
            if((buf[9]=='O')&&(buf[10]=='N')){
                sendHeader(sn);
                sendData(sn);
            }
            else{
                sendHeader(sn);
                sendData(sn);
            }
        }
        else{
            sendHeader(sn);
            sendData(sn);
        }
    }
 
    return 1;
}

int32_t WebServer(uint8_t sn, uint8_t* buf, uint16_t port)
{
   int32_t ret;
   uint16_t size = 0, sentsize=0, i=10000;
#ifdef _WEBSERVER_DEBUG_
   uint8_t destip[4];
   uint16_t destport;
#endif
	volatile uint8_t sr = getSn_SR(sn);
	if (sr == SOCK_CLOSED) debugMSG("Socket%d geschlossen!",sn); 
	else if (sr == SOCK_INIT) debugMSG("Socket%d Initial Mode!",sn);
	else if (sr == SOCK_LISTEN) debugMSG("Socket%d im Listen state!",sn);
	else if (sr == SOCK_ESTABLISHED) debugMSG("Socket%d Verbindung eingerichtet!",sn);
	else debugMSG("Socket%d Listening status %d!",sn, sr);

   switch(sr)
   {
      case SOCK_ESTABLISHED :
         if(getSn_IR(sn) & Sn_IR_CON)
         {
#ifdef _WEBSERVER_DEBUG_
            getSn_DIPR(sn, destip);
            destport = getSn_DPORT(sn);
 
            debugMSG("%d:Connected - %d.%d.%d.%d : %d\r\n",sn, destip[0], destip[1], destip[2], destip[3], destport);
#endif
            setSn_IR(sn,Sn_IR_CON);
         }
         while(i){
             i--;
         }
         if((size = getSn_RX_RSR(sn)) > 0) // Don't need to check SOCKERR_BUSY because it doesn't not occur.
         {
            if(size > ETH_MAX_BUF_SIZE) size = ETH_MAX_BUF_SIZE;
            ret = recv(sn, buf, size);
 
            if(ret <= 0) return ret;      // check SOCKERR_BUSY & SOCKERR_XXX. For showing the occurrence of SOCKERR_BUSY.
            sentsize = 0;
 
            debugMSG("HTTP Request received: \r\n%s\r\n", buf);
            proc_http(sn, (char *)buf);
 
         }
         break;
      case SOCK_CLOSE_WAIT :
#ifdef _WEBSERVER_DEBUG_
         debugMSG("%d:CloseWait\r\n",sn);
#endif
         if((ret = disconnect(sn)) != SOCK_OK) return ret;
#ifdef _WEBSERVER_DEBUG_
         debugMSG("%d:Socket Closed\r\n", sn);
#endif
         break;
      case SOCK_INIT :
#ifdef _WEBSERVER_DEBUG_
         debugMSG("%d:Listen, Web Server, port [%d]\r\n", sn, port);
#endif
         if( (ret = listen(sn)) != SOCK_OK) return ret;
         break;
      case SOCK_CLOSED:
#ifdef _WEBSERVER_DEBUG_
         debugMSG("%d:Web Server start\r\n",sn);
#endif
         if((ret = socket(sn, Sn_MR_TCP, port, 0x00)) != sn) return ret;
#ifdef _WEBSERVER_DEBUG_
         debugMSG("%d:Socket opened\r\n",sn);
#endif
         break;
      default:
         break;
   }
   return 1;
}



static void mainTest_task(void *params);

//! Handle for about main_TestTask
static xTaskHandle mainTestTask;


/** Initialisere Tasks
**  
**/
void testTasks_init(void)
{
	debugInit();
	

	
//	infoMSG("Info Beispiel");
//	debugMSG("Debug Beispiel");
//	warningMSG("Warnung Beispiel");
//	errorMSG("Error Beispiel");
//
//	infoMSG("Noch ein Paar Bytes als Test ausgabe im UART");
	
	
	xTaskCreate(mainTest_task,
		(const char *)"mainTest_task",
		1024,
		NULL,
		TEST_TASK_PRIORITY,
		&mainTestTask);
		
//		strcpy(send_dat ,"<!DOCTYPE html> <html> <body> <h1>This is heading 1</h1>	<h4>This is heading 4</h4> <h5>This is heading 5</h5> </body> </html>");
}


static void mainTest_task(void *params)
{
	volatile int8_t success;
	
	infoMSG("SAMD21 Projekt mit W5500 Integration");
	strcpy((char*) send_dat,"<!DOCTYPE html>		<html>		<body>		<h1>This is heading 1</h1>		<h2>This is heading 2</h2>		<h3>This is heading 3</h3>		<h4>This is heading 4</h4>		<h5>This is heading 5</h5>		<h6>This is heading 6</h6>		</body>		</html>");
	
	
	reg_wizchip_spi_cbfunc(spiReadByte, spiWriteByte);
	reg_wizchip_cs_cbfunc(csEnable,csDisable);
	
	configure_spi_master();	
	W5x00Initialze();
//	wizchip_init();
	volatile wiz_NetInfo info;
	wizchip_setnetinfo(&gWIZNETINFO);
//	wizchip_getnetinfo(&info);
	
 	setSHAR(&(gWIZNETINFO.mac));
 	setGAR(&gWIZNETINFO.gw);
 	setSUBR(&gWIZNETINFO.sn);
 	setSIPR(&gWIZNETINFO.ip);
	
	getSHAR(&gWIZNETINFO.mac);
	getGAR(&gWIZNETINFO.gw);
	getSUBR(&gWIZNETINFO.sn);
	getSIPR(&gWIZNETINFO.ip);
	
	
	uint8_t tmpstr[6] = {0,};
	ctlwizchip(CW_GET_ID,(void*)tmpstr);
	infoMSG("WizchipName: %s",tmpstr);
	
	uint8_t tmpstr2[6] = {0,};
	ctlwizchip(CW_RESET_WIZCHIP,(void*)tmpstr2);
	
	for (int i=0; i<8; i++) 
	{
		uint8_t cntl_byte = (0x0C + (i<<5));
		
		spiWriteByte(0x1E >> 8);
		spiWriteByte(0x1E & 0xFF);
		spiWriteByte(cntl_byte);
		spiWriteByte(2);
		
		spiWriteByte(0x1F >> 8);
		spiWriteByte(0x1F & 0xFF);
		spiWriteByte(cntl_byte);
		spiWriteByte(2);
	}
	
	
//	setSHAR(&(gWIZNETINFO.mac));
//	setGAR(&gWIZNETINFO.gw);
//	setSUBR(&gWIZNETINFO.sn);
//	setSIPR(&gWIZNETINFO.ip);

		
	
	
	uint8_t sn = 1;
	
	for(uint8_t i = 0; i<8; i++)
	{
		success = socket(i,Sn_MR_TCP,PORT_WEBSERVER,0);
		success = listen(i);
	}
	

	
	
	success = getSn_SR(sn);
	if (success == SOCK_CLOSED) debugMSG("Socket%d geschlossen!",sn);
	else if (success == SOCK_INIT) debugMSG("Socket%d Initial Mode!",sn);
	else if (success == SOCK_LISTEN) debugMSG("Socket%d im Listen state!",sn);
	else if (success == SOCK_ESTABLISHED) debugMSG("Socket%d Verbindung eingerichtet!",sn);
	else debugMSG("Socket%d status %d!",sn, success);
	

	success = socket(sn,Sn_MR_TCP,PORT_WEBSERVER,0);
	if (success == sn) debugMSG("Socket Erstellung erfolgreich!");
	else if (success == SOCKERR_SOCKNUM) errorMSG("Socket Invalid socket number !");
	else if (success == SOCKERR_SOCKMODE) errorMSG("Socket Not support socket mode!");
	else if (success == SOCKERR_SOCKFLAG) errorMSG("Socket Invaild socket flag!");	
	else debugMSG("Socket%d status %d!",sn, success);
	
	success = getSn_SR(sn);
	if (success == SOCK_CLOSED) debugMSG("Socket%d geschlossen!",sn);
	else if (success == SOCK_INIT) debugMSG("Socket%d Initial Mode!",sn);
	else if (success == SOCK_LISTEN) debugMSG("Socket%d im Listen state!",sn);
	else if (success == SOCK_ESTABLISHED) debugMSG("Socket%d Verbindung eingerichtet!",sn);
	else debugMSG("Socket%d status %d!",sn, success);

	success = listen(0);
	volatile uint8_t test = SOCKERR_SOCKINIT;
	if (success == SOCK_OK) debugMSG("Socket Listening erfolgreich!");
	else if (success == SOCKERR_SOCKINIT) errorMSG("Socket Listening socket not initiated!");
	else if (success == SOCKERR_SOCKCLOSED) errorMSG("Socket Listening socket closed!");
	else if (success == SOCKERR_SOCKOPT) errorMSG("Socket Listening invalid Socket option!");
	else if (success == SOCKERR_SOCKMODE) errorMSG("Socket Listening Socketmode invalid!");
	else debugMSG("Socket%d listening status %d!",sn, success);
	
	
	success = getSn_SR(sn);
	if (success == SOCK_CLOSED) debugMSG("Socket%d geschlossen!",sn);
	else if (success == SOCK_INIT) debugMSG("Socket%d Initial Mode!",sn);
	else if (success == SOCK_LISTEN) debugMSG("Socket%d im Listen state!",sn);
	else if (success == SOCK_ESTABLISHED) debugMSG("Socket%d Verbindung eingerichtet!",sn);
	else debugMSG("Socket%d status %d!",sn, success);

	
	print_network_information();

	
	while(1)
	{
		//memset(&gWIZNETINFO,0,sizeof(gWIZNETINFO));
		//wizchip_getnetinfo(&gWIZNETINFO);

		
		//zr++;
		
		//port_pin_toggle_output_level(PIN_PA17);
		//delay_ms(200);
		//print_network_information();
		
		WebServer(0, ethBuf1, PORT_WEBSERVER);
		
		vTaskDelay(500); // kleine Pause um nicht die ganze zeit aktiv zu warten

		
	}
}



inline void csEnable(void)
{
	spi_select_slave(&spi_master_instance, &slave, true);
}

inline void csDisable(void)
{
	spi_select_slave(&spi_master_instance, &slave, false);
}


uint8_t spiReadByte(void)
{
	volatile uint16_t zr;
	volatile uint16_t zr2 = 0x00000000;
	

//	spi_write(&spi_master_instance,&zr);
//	//while(!spi_is_ready_to_read(&spi_master_instance)){};
//	spi_read(&spi_master_instance,&zr);
	
	spi_transceive_wait(&spi_master_instance,zr2,&zr);
	return (uint8_t)(zr & 0xff);
}

void spiWriteByte(uint8_t byte)
{
	volatile uint16_t zr;
	spi_transceive_wait(&spi_master_instance,(uint8_t)byte,&zr);
//		spi_write(&spi_master_instance,(uint8_t)byte);
}


void W5x00Initialze(void)
{
	intr_kind temp;
	int8_t phy_link =0;

	unsigned char W5x00_AdrSet[2][8] = {{2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};
	temp = IK_DEST_UNREACH;
	
	int8_t success = ctlwizchip(CW_INIT_WIZCHIP,(void*)W5x00_AdrSet);
	debugMSG("CW_INIT_WIZCHIP : %d",success);
	if(success == -1)
	{
		errorMSG("W5x00 initialized fail.\r\n");
	}
	success = ctlwizchip(CW_SET_INTRMASK,&temp);
	debugMSG("CW_SET_INTRMASK : %d",success);
	if(success == -1)
	{
		infoMSG("W5x00 interrupt\r\n");
	}
	do{//check phy status.
		if(ctlwizchip(CW_GET_PHYLINK,&phy_link) == -1){
			errorMSG("Unknown PHY link status.\r\n");
			delay_ms(10);
		}

	}while(phy_link == PHY_LINK_OFF);
	infoMSG("W5500 initialisiert");
}

void print_network_information(void)
{
	memset(&gWIZNETINFO,0,sizeof(gWIZNETINFO));

	wizchip_getnetinfo(&gWIZNETINFO);
	infoMSG("MAC Address : %02x:%02x:%02x:%02x:%02x:%02x",gWIZNETINFO.mac[0],gWIZNETINFO.mac[1],gWIZNETINFO.mac[2],gWIZNETINFO.mac[3],gWIZNETINFO.mac[4],gWIZNETINFO.mac[5]);
	infoMSG("IP  Address : %d.%d.%d.%d",gWIZNETINFO.ip[0],gWIZNETINFO.ip[1],gWIZNETINFO.ip[2],gWIZNETINFO.ip[3]);
	infoMSG("Subnet Mask : %d.%d.%d.%d",gWIZNETINFO.sn[0],gWIZNETINFO.sn[1],gWIZNETINFO.sn[2],gWIZNETINFO.sn[3]);
	infoMSG("Gateway     : %d.%d.%d.%d",gWIZNETINFO.gw[0],gWIZNETINFO.gw[1],gWIZNETINFO.gw[2],gWIZNETINFO.gw[3]);
	infoMSG("DNS Server  : %d.%d.%d.%d",gWIZNETINFO.dns[0],gWIZNETINFO.dns[1],gWIZNETINFO.dns[2],gWIZNETINFO.dns[3]);
}