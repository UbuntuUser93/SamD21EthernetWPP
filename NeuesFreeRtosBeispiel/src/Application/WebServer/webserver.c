/*
 * werbserver.c
 *
 * Created: 25.02.2021 22:53:55
 *  Author: KennisTower
 */ 

#define MAINPAGESIZE 512

char MainPage[MAINPAGESIZE];

uint16_t iteratorMainPage;

void Webserver_init()
{
	iteratorMainPage = 0;
	const char Header[] = "<!DOCTYPE html>		<html>		<body>"
	strcpy(MainPage,Header);
	iteratorMainPage += sizeof(Header);
}

void Webserver_print(char* format,...)
{
	va_list argptr;
	va_start(argptr, format);
	
	strcpy(new_msg,INFO_PRAEFIX);
	vsprintf((new_msg + sizeof(INFO_PRAEFIX) - 1),format,argptr);


	va_end(argptr);
}

void Webserver_addMSG()
{
	
}