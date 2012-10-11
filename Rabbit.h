
#ifndef	RABBIT_H
#define RABBIT_H

#include "SPI.h"			// needed for Arduino versions later than 0018
#include "Ethernet.h"

#define NBDATA	3

typedef struct {	//	SRabbit
	int statusLed;
	int errorLed;
	int button1;
	int button2;
	int button3;
	int button4;
	unsigned long ulDelay;
	byte mac[6];
}SRabbit;

#endif
