
#include "Halogene.h"
#include <SPI.h>          // needed for Arduino versions later than 0018

SHalogene sHalogene = { '2', 10, 11, EStatus_Off, A0, 9, 1023, 0, 5,
						{ 0, 30, 52, 71, 89, 105, 121, 136, 151, 165, 179, 192, 205, 218, 230, 243, 255 },
						{ 0, 3, 10, 20, 31, 43, 57, 73, 89, 107, 125, 144, 165, 186, 208, 231, 255 } };
SMessage sMessIn, sMessOut;
unsigned int manuelValue = 0, lastOutputValue = 0;
unsigned int previousValues[NB_PREVIOUSVALUES], previousCpt;
unsigned long ulTimer = 0;

void setup() {
	pinMode(sHalogene.statusLed, OUTPUT);
	pinMode(sHalogene.errorLed, OUTPUT);
	digitalWrite(sHalogene.statusLed, HIGH);
	digitalWrite(sHalogene.errorLed, HIGH);

	Serial.begin(9600);
	for(previousCpt = 0; previousCpt < NB_PREVIOUSVALUES; previousCpt++)	previousValues[previousCpt] = analogRead(sHalogene.analogInPin);

	digitalWrite(sHalogene.statusLed, LOW);
	digitalWrite(sHalogene.errorLed, LOW);
}

void loop() {
	static unsigned int sensorValue = 0, outputValue = 0;
	unsigned int temp = 0, cpt = 0;

	if(Serial.available() > 0) {	//	Reception message serie
		blink(sHalogene.statusLed, 100, 1);
		if(LireMessageSerie() == sHalogene.nModule) {
			switch(sMessIn.eTypeTrame) {	//	Type de trame
				case ETypeTrame_Polling :
					memcpy(&sMessOut, &sMessIn, sizeof(SMessage));
					sMessOut.eTypeMsg = sHalogene.eStatus + 0x30;
					if(sHalogene.eStatus == EStatus_Auto) {
						sMessOut.lData[0] = lastOutputValue / 100 + 0x30;
						sMessOut.lData[1] = lastOutputValue % 100 / 10 + 0x30;
						sMessOut.lData[2] = lastOutputValue % 10 + 0x30;
						sMessOut.lData[3] = 0;
					}
					if(sHalogene.eStatus == EStatus_Manuel) {
						sMessOut.lData[0] = manuelValue / 100 + 0x30;
						sMessOut.lData[1] = manuelValue % 100 / 10 + 0x30;
						sMessOut.lData[2] = manuelValue % 10 + 0x30;
						sMessOut.lData[3] = 0;
					}
					sMessOut.nModule = sMessIn.nEmetteur;
					sMessOut.nEmetteur = sHalogene.nModule;
					EnvoiMessageSerie();
					break;
					
				case ETypeTrame_Commande :
					memcpy(&sMessOut, &sMessIn, sizeof(SMessage));
					sMessOut.eTypeTrame = ETypeTrame_Ok;
					switch(sMessIn.eTypeMsg) {	//	Type de message
						case ETypeMsg_Auto :
							sHalogene.eStatus = EStatus_Auto;
							break;
							
						case ETypeMsg_On :
							sHalogene.eStatus = EStatus_On;
							break;
							
						case ETypeMsg_Off :
							sHalogene.eStatus = EStatus_Off;
							break;
							
						case ETypeMsg_Manuel :
 							temp = (sMessIn.lData[0] - 0x30) * 100 + (sMessIn.lData[1] - 0x30) * 10 + (sMessIn.lData[2] - 0x30);
							if(temp >= 0 && temp <= 100) {
								manuelValue = temp;
								sHalogene.eStatus = EStatus_Manuel;
							}
							else {
								blink(sHalogene.errorLed, 100, 2);
							}
							break;
							
						default :
							sMessOut.eTypeTrame = ETypeTrame_Nok;
							blink(sHalogene.errorLed, 100, 2);
							break;
					}
					sMessOut.nModule = sMessIn.nEmetteur;
					sMessOut.nEmetteur = sHalogene.nModule;
					EnvoiMessageSerie();
					break;
					
				case ETypeTrame_Ok :
					break;
					
				case ETypeTrame_Nok :
					break;
					
				default :
					sMessOut.nModule = sMessIn.nEmetteur;
					sMessOut.eTypeTrame = ETypeTrame_Nok;
					sMessOut.nEmetteur = sHalogene.nModule;
					EnvoiMessageSerie();
					blink(sHalogene.errorLed, 100, 1);
					break;
			}
		}
	}
	
	//	Lecture photoresistance
	sensorValue = analogRead(sHalogene.analogInPin);/*
	Serial.print(sensorValue);
	Serial.print("   ");//*/

	//	Sauvegarde de la valeur
	if(previousCpt >= NB_PREVIOUSVALUES)	previousCpt = 0;
	previousValues[previousCpt++] = sensorValue;

	//	Mise a jour des extremas
	temp = 0;
	for(cpt = 0; cpt < NB_PREVIOUSVALUES; cpt++)	temp += previousValues[cpt];
	sensorValue = temp / NB_PREVIOUSVALUES;
	if(sensorValue < sHalogene.nMin) {
		sHalogene.nMin = sensorValue;
		blink(sHalogene.statusLed, 50, 1);
	}
	if(sensorValue > sHalogene.nMax) {
		sHalogene.nMax = sensorValue;
		blink(sHalogene.statusLed, 50, 1);
	}/*
	Serial.print(sHalogene.nMin);
	Serial.print("   ");
	Serial.print(sHalogene.nMax);
	Serial.print("   ");//*/

	switch(sHalogene.eStatus) {		//	Ecriture commande
		case EStatus_Auto:
			//	Formatage de la sortie
			outputValue = map(sensorValue, sHalogene.nMin, sHalogene.nMax, 0, 255);/*
			Serial.print(outputValue);
			Serial.print("   ");
			Serial.print(lastOutputValue);
			Serial.print("   ");//*/

			if(lastOutputValue == 1 && outputValue <= ( sHalogene.nSeuils[lastOutputValue - 1] + sHalogene.nDelta )) {
				outputValue = sHalogene.nOutput[--lastOutputValue];
			}
			else if(lastOutputValue > 1 && outputValue <= ( sHalogene.nSeuils[lastOutputValue - 1] - sHalogene.nDelta )) {
				outputValue = sHalogene.nOutput[--lastOutputValue];
			}
			else if(lastOutputValue == NB_SEUILS - 2 && outputValue >= ( sHalogene.nSeuils[lastOutputValue + 1] - sHalogene.nDelta )) {
				outputValue = sHalogene.nOutput[++lastOutputValue];
			}
			else if(lastOutputValue < NB_SEUILS - 1 && outputValue >= ( sHalogene.nSeuils[lastOutputValue + 1] + sHalogene.nDelta )) {
				outputValue = sHalogene.nOutput[++lastOutputValue];
			}
			else {
				outputValue = sHalogene.nOutput[lastOutputValue];
			}/*
			Serial.print(outputValue);
			Serial.println("   ");//*/

			//	Envoi de la commande
			analogWrite(sHalogene.analogOutPin, outputValue);
			break;

		case EStatus_On:
			analogWrite(sHalogene.analogOutPin, 255);
			break;
			
		case EStatus_Off:
			analogWrite(sHalogene.analogOutPin, 0);
			break;
			
		case EStatus_Manuel:
			analogWrite(sHalogene.analogOutPin, manuelValue * 255 / 100);
			break;
			
		default :
			break;
	}

	if(millis()-ulTimer <= 0 || millis()-ulTimer > 3600000) {
		sHalogene.nMin++;
		sHalogene.nMax--;
		ulTimer = millis();
	}

	delay(500);	//	Periode raflaichissement
}

unsigned char LireMessageSerie() {
	unsigned short cpt = 0;

	sMessIn.nModule = Serial.read();
	sMessIn.eTypeTrame = Serial.read();
	sMessIn.eTypeMsg = Serial.read();
	sMessIn.nEmetteur = Serial.read();
	for(cpt=0; cpt<NBDATA; cpt++) {
		sMessIn.lData[cpt] = Serial.read(); }
	sMessIn.lData[NBDATA+1] = '\0';
	sMessIn.nCrc[0] = Serial.read();
	sMessIn.nCrc[1] = Serial.read();
	sMessIn.nCrc[2] = '\0';
		
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

void CalculCrcSerie() {
	word wCrc = 0;
	unsigned short cpt;
	
	wCrc ^= word(sMessOut.nModule, sMessOut.eTypeTrame);
	wCrc ^= word(sMessOut.eTypeMsg, sMessOut.nEmetteur);
	for(cpt=4; cpt<4+sizeof(sMessOut.lData)-1; cpt+=2) {
		wCrc ^= word(sMessOut.lData[cpt], sMessOut.lData[cpt+1]); }
	
	sMessOut.nCrc[0] = highByte(wCrc);
	sMessOut.nCrc[1] = lowByte(wCrc);
}

bool CheckCrcSerie() {
	word wCrc = 0;
	unsigned short cpt;
	
	wCrc ^= word(sMessIn.nModule, sMessIn.eTypeTrame);
	wCrc ^= word(sMessIn.eTypeMsg, sMessIn.nEmetteur);
	for(cpt=4; cpt<4+sizeof(sMessIn.lData)-1; cpt+=2) {
		wCrc ^= word(sMessIn.lData[cpt], sMessIn.lData[cpt+1]); }
	
	if(sMessIn.nCrc[0] == highByte(wCrc) && sMessIn.nCrc[1] == lowByte(wCrc)) {
		return true; }
	return false;
}

void blink(int ledPin, unsigned long ulDelay, int nCpt) {
	digitalWrite(ledPin, HIGH);
	delay(ulDelay);				//  en ms
	digitalWrite(ledPin, LOW);
	while(--nCpt>0) {
		delay(ulDelay);
		digitalWrite(ledPin, HIGH);
		delay(ulDelay);
		digitalWrite(ledPin, LOW);
	}
}
