
#ifndef	HALOGENE_H
#define HALOGENE_H

#include "SPI.h"			// needed for Arduino versions later than 0018

#define	NB_PREVIOUSVALUES	64
#define	NB_SEUILS	17
#define NBDATA	3

typedef struct {	//	SHalogene
	unsigned char nModule;
	unsigned int statusLed;
	unsigned int errorLed;
	unsigned int eStatus;
	const int analogInPin;
	const int analogOutPin;
	unsigned int nMin;
	unsigned int nMax;
	const int nDelta;
	const int nSeuils[NB_SEUILS];
	const int nOutput[NB_SEUILS];
}SHalogene;

typedef struct {	//	SMessage
	char nModule;
	char eTypeTrame;
	char eTypeMsg;
	char nEmetteur;
	char lData[NBDATA+1];
	char nCrc[3];
}SMessage;

typedef enum {	//	ETypeTrame
	ETypeTrame_Polling = '0',
	ETypeTrame_Commande = '1',
	ETypeTrame_Ok = '2',
	ETypeTrame_Nok = '3',
	ETypeTrame_Dernier
}ETypeTrame;

typedef enum {	//	ETypeMsg
	ETypeMsg_Auto = '0',
	ETypeMsg_On = '1',
	ETypeMsg_Off = '2',
	ETypeMsg_Manuel = '3',
	ETypeMsg_Dernier,
}ETypeMsg;

typedef enum {	//	EStatus
	EStatus_Auto = 0,
	EStatus_On = 1,
	EStatus_Off = 2,
	EStatus_Manuel = 3,
	EStatus_Dernier
}EStatus;

#endif
