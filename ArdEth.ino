
#include "ArdEth.h"
#include "SPI.h"               // needed for Arduino versions later than 0018
#include "Ethernet.h"
//#include <WebSocket.h>

SRouteur sRouteur = { '1', 8, 9, { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }, 8888 };
SMessage sMessIn, sMessOut, sMessAtt;
EthernetUDP Udp;
//short nCount = 0;
//static unsigned int halogeneStatus = EStatus_Auto;
//static unsigned int halogeneManuelValue = 0;
EthernetServer server = EthernetServer(sRouteur.localPort);

void setup() {
	pinMode(sRouteur.statusLed, OUTPUT);
	pinMode(sRouteur.errorLed, OUTPUT);
	digitalWrite(sRouteur.statusLed, HIGH);
	digitalWrite(sRouteur.errorLed, HIGH);

	Ethernet.begin(sRouteur.mac);
	server.begin();
	Serial.begin(9600);

	digitalWrite(sRouteur.statusLed, LOW);
	digitalWrite(sRouteur.errorLed, LOW);
}

void loop() {
	TraiterMessageTcp();
	TraiterMessageSerie();
	delay(10);
}

void TraiterMessageTcp() {
	EthernetClient client = server.available();
	if(client == true) {
		LireBufferTcp(client, client.available());
		LireMessageBuffer();
//		Serial.println(sRouteur.packetBuffer);
		if(sMessIn.nModule == sRouteur.nModule) {
			EnvoiMessageTcp();
			blink(sRouteur.statusLed, 100, 1);
		}
		else if(sMessIn.nEmetteur == '0') {
			memcpy(&sMessOut, &sMessIn, sizeof(SMessage));
//			sMessOut.nEmetteur = sRouteur.nModule;
			EnvoiMessageSerie();
		}
		blink(sRouteur.statusLed, 100, 1);
	}
}

void LireBufferTcp(EthernetClient client, long nBytes) {
	long i = 0;
	do{
		sRouteur.packetBuffer[i++] = client.read();
	} while(--nBytes > 0 && sRouteur.packetBuffer[i] != -1);
	sRouteur.packetBuffer[i] = 0;
}

unsigned char LireMessageBuffer() {
	short i = 0;
	memset(&sMessIn, 0, sizeof(sMessIn));
	sMessIn.nModule = sRouteur.packetBuffer[i++];
	sMessIn.eTypeTrame = sRouteur.packetBuffer[i++];
	sMessIn.eTypeMsg = sRouteur.packetBuffer[i++];
	sMessIn.nEmetteur = sRouteur.packetBuffer[i++];
	memcpy(sMessIn.lData, &sRouteur.packetBuffer[i], NBDATA);
	sMessIn.lData[NBDATA] = 0;	i += NBDATA;
	memcpy(sMessIn.nCrc, &sRouteur.packetBuffer[i], sizeof(sMessIn.nCrc));
	sMessIn.nCrc[NBCRC] = 0;	i += NBCRC;
	return sMessIn.nModule;
}

void EnvoiMessageSerie() {
	Serial.print(sMessOut.nModule);
	Serial.print(sMessOut.eTypeTrame);
	Serial.print(sMessOut.eTypeMsg);
	Serial.print(sMessOut.nEmetteur);
	Serial.print(sMessOut.lData);
	Serial.println(sMessOut.nCrc);
}

void TraiterMessageSerie() {
	if(Serial.available() > 0) {
		delay(10);
		LireBufferSerie(Serial.available());
		switch(LireMessageBuffer()) {
			case '0':
				memcpy(&sMessAtt, &sMessIn, sizeof(SMessage));
			break;
/*			case '1':
				switch(sMessIn.eTypeTrame) {
					case ETypeTrame_Ok :
						if(sMessIn.nEmetteur == '2') { halogeneStatus = sMessIn.eTypeMsg; }
						if(sMessIn.eTypeMsg == ETypeMsg_Manuel) {
							halogeneManuelValue = (sMessIn.lData[0] - 0x30) * 100 + (sMessIn.lData[1] - 0x30) * 10 + (sMessIn.lData[2] - 0x30);
						}
						blink(sRouteur.statusLed, 100, 1);
						break;

					case ETypeTrame_Nok :
						if(sMessIn.nEmetteur == '2') { halogeneStatus = sMessIn.eTypeMsg; }
						if(sMessIn.eTypeMsg == ETypeMsg_Manuel) {
							halogeneManuelValue = (sMessIn.lData[0] - 0x30) * 100 + (sMessIn.lData[1] - 0x30) * 10 + (sMessIn.lData[2] - 0x30);
						}
						blink(sRouteur.errorLed, 100, 1);
						if(nCount < 4) {
							if(nCount++ < 3) {
								EnvoiMessageSerie();
							}
							else {
								memcpy(&sMessAtt, &sMessOut, sizeof(SMessage));
							}
						}
						break;

					default :
						blink(sRouteur.errorLed, 100, 1);
						if(nCount++ < 3) {
							sMessOut.nModule = sMessIn.nEmetteur;
							sMessOut.eTypeTrame = ETypeTrame_Nok;
							sMessOut.nEmetteur = sRouteur.nModule;
							EnvoiMessageSerie();
						}
						if(nCount == 4) {
							memcpy(&sMessAtt, &sMessOut, sizeof(SMessage));
						}
						break;
				}
			break;*/
			default:
				Serial.println(sMessIn.nModule);
				blink(sRouteur.errorLed, 100, 1);
			break;
		}
	}
}

void LireBufferSerie(long nBytes) {
	long i = 0;
	do{
		sRouteur.packetBuffer[i++] = Serial.read();
	} while(--nBytes > 0 && sRouteur.packetBuffer[i] != -1);
	sRouteur.packetBuffer[i] = 0;
	Serial.flush();
	Serial.println(sRouteur.packetBuffer);
}

void EnvoiMessageTcp() {
	short i = 0;
	server.write(sMessOut.nModule);
	server.write(sMessOut.eTypeTrame);
	server.write(sMessOut.eTypeMsg);
	server.write(sMessOut.nEmetteur);
	while(i < NBDATA) {
		server.write(sMessOut.lData[i++]);
	}	i = 0;
	while(i < NBCRC) {
		server.write(sMessOut.nCrc[i++]);
	}	
}

/*void AttendreReponseSerie() {
	while(!TraiterMessageSerie());     //     Attente reponse
}//*/

/*void CalculCrcSMessOut() {
	word wCrc = 0;
	unsigned short cpt;

	wCrc ^= word(sMessOut.nModule, sMessOut.eTypeTrame);
	wCrc ^= word(sMessOut.eTypeMsg, sMessOut.nEmetteur);
	for(cpt=4; cpt<4+sizeof(sMessOut.lData)-1; cpt+=2) {
		wCrc ^= word(sMessOut.lData[cpt], sMessOut.lData[cpt+1]);
	}
	sMessOut.nCrc[0] = highByte(wCrc);
	sMessOut.nCrc[1] = lowByte(wCrc);
}//*/

/*bool CheckCrcSerieSMessIn() {
	word wCrc = 0;
	unsigned short cpt;

	wCrc ^= word(sMessIn.nModule, sMessIn.eTypeTrame);
	wCrc ^= word(sMessIn.eTypeMsg, sMessIn.nEmetteur);
	for(cpt=4; cpt<4+sizeof(sMessIn.lData)-1; cpt+=2) {
		wCrc ^= word(sMessIn.lData[cpt], sMessIn.lData[cpt+1]);
	}
	if(sMessIn.nCrc[0] == highByte(wCrc) && sMessIn.nCrc[1] == lowByte(wCrc)) {
		return true;
	}
	return false;
}//*/

void blink(int ledPin, unsigned long ulDelay, int nCpt) {
	digitalWrite(ledPin, HIGH);
	delay(ulDelay);				//		en ms
	digitalWrite(ledPin, LOW);
	while(--nCpt>0) {
		delay(ulDelay);
		digitalWrite(ledPin, HIGH);
		delay(ulDelay);
		digitalWrite(ledPin, LOW);
	}
}
