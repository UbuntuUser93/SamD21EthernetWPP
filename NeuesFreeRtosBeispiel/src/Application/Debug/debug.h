/*
 * debug.h
 *
 * Created: 13.02.2021 20:39:36
 *  Author: KennisTower
 */ 


#ifndef DEBUG_H_
#define DEBUG_H_


#include "stdio.h"
#include "stdarg.h"

#include "FreeRTOS.h"
#include "task.h"
#include "usart.h"

#define ESC "\033"
#define CLR_SEQ "[2J"
#define CURSOR_RESET_SEQ "[0;0H"
#define INFO_COLOR_SEQ "[30;102m"
#define DEBUG_COLOR_SEQ "[30;103m"
#define WARNING_COLOR_SEQ "[30;105m"
#define ERROR_COLOR_SEQ "[30;101m"
#define DEFAULT_COLOR_SEQ "[40;37m"


#define INCLUDE_vTaskDelay 1

#define INFO_PRAEFIX (ESC INFO_COLOR_SEQ "Info:   " ESC DEFAULT_COLOR_SEQ " ")
#define DEBUG_PRAEFIX (ESC DEBUG_COLOR_SEQ "Debug:  " ESC DEFAULT_COLOR_SEQ " ")
#define WARNING_PRAEFIX (ESC WARNING_COLOR_SEQ "Warnung:" ESC DEFAULT_COLOR_SEQ " ")
#define ERROR_PRAEFIX (ESC ERROR_COLOR_SEQ "Error:  " ESC DEFAULT_COLOR_SEQ " ")


struct usart_module usart_instance;


void infoMSG(char* format,...);
//
// Funktion kann verwendet werden um eine Nachricht mit dem Debug Praefix per UART auszugeben
// Am ende des angegebenen Textes wird automatisch ein \n angehangen!
//
void debugMSG(char *,...);
/************************************************************************/
/* Funktion fügt die Daten aus dem Char Pointer zu dem Sendebuffer hinzu
/*   Char Pointer "string" muss mit einer 0 enden und wird dann mit einem \n von dieser Funktion abgeschlossen!
/*
/************************************************************************/
void warningMSG(char* format,...);
void errorMSG(char* format,...);




void addMSG(char *);

void debugInit();

static void debugTask(void *params);



int _write(int file, char *ptr, int len);
int _read(int file, char *ptr, int len);



#endif /* DEBUG_H_ */