
#ifndef	ARDETH_H
#define	ARDETH_H

#include "SPI.h"               // needed for Arduino versions later than 0018
#include "Ethernet.h"

#define	BUFFERSIZE	1024
#define NBDATA		3
#define NBCRC		2

typedef struct {		//		SRouteur
	char nModule;
	int statusLed;
	int errorLed;
	byte mac[6];
	unsigned int localPort;
	char packetBuffer[BUFFERSIZE];
}SRouteur;

typedef struct {		//		SMessage
	char nModule;
	char eTypeTrame;
	char eTypeMsg;
	char nEmetteur;
	char lData[NBDATA+1];
	char nCrc[NBCRC+1];
}SMessage;

typedef enum {		//		ETypeTrame
	ETypeTrame_Polling = '0',
	ETypeTrame_Commande = '1',
	ETypeTrame_Ok = '2',
	ETypeTrame_Nok = '3',
	ETypeTrame_Dernier
}ETypeTrame;

typedef enum {		//		ETypeMsg
	ETypeMsg_Auto = '0',
	ETypeMsg_On = '1',
	ETypeMsg_Off = '2',
	ETypeMsg_Manuel = '3',
	ETypeMsg_Dernier,
}ETypeMsg;

typedef enum {		//		EStatus
	EStatus_Auto = 0,
	EStatus_On = 1,
	EStatus_Off = 2,
	EStatus_Manuel = 3,
	EStatus_Dernier
}EStatus;

#endif
