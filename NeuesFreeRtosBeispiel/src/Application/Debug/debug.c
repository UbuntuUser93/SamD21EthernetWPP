/*
 * debug.c
 *
 * Created: 13.02.2021 20:39:22
 *  Author: KennisTower
 */ 

#include "Application/Debug/debug.h"

#include "string.h"
#include "usart_interrupt.h"
#include "status_codes.h"

#include "stdio.h"
#include "stdarg.h"

#define MSG_BUFFER_LENGTH	512
#define NEW_MSG_LENGTH		128
#define DEBUG_TASK_PRIORITY (tskIDLE_PRIORITY + 2)


volatile char MSG_Buffer[MSG_BUFFER_LENGTH];
volatile uint16_t MSG_iterator; // zeigt immer auf den nächsten, noch nicht beschriebenen Speicher im Ringbuffer MSG_Buffer
uint16_t SEND_iterator; // zeigt welches Byte gerade gesendet werden soll

volatile char new_msg[NEW_MSG_LENGTH];

static xTaskHandle debugTaskHandle;



void infoMSG(char* format,...)
{
	va_list argptr;
	va_start(argptr, format);
	
	strcpy(new_msg,INFO_PRAEFIX);
	vsprintf((new_msg + sizeof(INFO_PRAEFIX) - 1),format,argptr);
	addMSG(new_msg);

	va_end(argptr);
}
void debugMSG(char* format,...)
{
	va_list argptr;
	va_start(argptr, format);
	
	strcpy(new_msg,DEBUG_PRAEFIX);
	vsprintf((new_msg + sizeof(DEBUG_PRAEFIX) - 1),format,argptr);
	addMSG(new_msg);

	va_end(argptr);
}
void warningMSG(char* format,...)
{
	va_list argptr;
	va_start(argptr, format);
	
	strcpy(new_msg,WARNING_PRAEFIX);
	vsprintf((new_msg + sizeof(WARNING_PRAEFIX) - 1),format,argptr);
	addMSG(new_msg);

	va_end(argptr);
}
void errorMSG(char* format,...)
{
	va_list argptr;
	va_start(argptr, format);
	
	strcpy(new_msg,ERROR_PRAEFIX);
	vsprintf((new_msg + sizeof(ERROR_PRAEFIX) - 1),format,argptr);
	addMSG(new_msg);

	va_end(argptr);
}


void addMSG(char* string)
{
	uint8_t i = 0;
	taskENTER_CRITICAL();
	while(string[i] != 0)
	{
		MSG_Buffer[MSG_iterator] = string[i];
		string[i] = 0;
		MSG_iterator++;
		i++;
		if(MSG_iterator >= MSG_BUFFER_LENGTH) MSG_iterator = 0;
		if (i >= NEW_MSG_LENGTH) break; // kein \n gefunden!
	}
	MSG_Buffer[MSG_iterator] =0x0A; // next line
	MSG_iterator++;
	MSG_Buffer[MSG_iterator] =0x0D; // Cursor to first Row
	MSG_iterator++;
	if(MSG_iterator >= MSG_BUFFER_LENGTH) MSG_iterator = 0;
	taskEXIT_CRITICAL();

}


void debugInit()
{
	MSG_iterator = 0;
	SEND_iterator = 0;
	
	strcpy(new_msg,ESC CLR_SEQ ESC CURSOR_RESET_SEQ);
	addMSG(new_msg);

	struct usart_config config_usart;

	usart_get_config_defaults(&config_usart);

	config_usart.baudrate    = 115200;
	config_usart.mux_setting = USART_RX_1_TX_0_XCK_1;
	config_usart.pinmux_pad0 = PINMUX_PB08D_SERCOM4_PAD0;
	config_usart.pinmux_pad1 = PINMUX_PB09D_SERCOM4_PAD1;
	config_usart.pinmux_pad2 = PINMUX_UNUSED;
	config_usart.pinmux_pad3 = PINMUX_UNUSED;
	while (usart_init(&usart_instance, SERCOM4, &config_usart) != STATUS_OK){};
	usart_enable(&usart_instance);
	

	
	xTaskCreate(debugTask,
		(const char *)"Debug Task",
		2024,
		NULL,
		DEBUG_TASK_PRIORITY,
		&debugTaskHandle);
	
}




static void debugTask(void *params)
{

	
	while(1)
	{
		if(SEND_iterator != MSG_iterator)
		{
			taskENTER_CRITICAL();
			volatile enum status_code code;
			uint16_t dat = MSG_Buffer[SEND_iterator];
			code = usart_write_wait(&usart_instance,dat);
			MSG_Buffer[SEND_iterator] = 0;
			SEND_iterator++;	
			if(SEND_iterator >= MSG_BUFFER_LENGTH) SEND_iterator = 0;
			taskEXIT_CRITICAL();
		}
		else
		{
			vTaskDelay(50); // kleine Pause um nicht die ganze zeit aktiv zu warten //TODO führt zu Hardfault Handler!
		}
	}
}








//Altlast:
int _write(int file, char *ptr, int len)
{
	return 0;
}
int _read(int file, char *ptr, int len)
{
}
