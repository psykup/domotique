
#include "Rabbit.h"
#include <SPI.h>			// needed for Arduino versions later than 0018
#include <Ethernet.h>

SRabbit sRabbit = { 8, 9, 4, 5, 6, 7, 100, { 0xF0, 0xC6, 0x12, 0x84, 0x88, 0x17 } };
String inString = String(35);
EthernetServer server(80);

void setup() {	//	Initialisation
	pinMode(sRabbit.statusLed, OUTPUT);
	pinMode(sRabbit.errorLed, OUTPUT);
	pinMode(sRabbit.button1, OUTPUT);
	pinMode(sRabbit.button2, OUTPUT);
	pinMode(sRabbit.button3, OUTPUT);
	pinMode(sRabbit.button4, OUTPUT);
	
	digitalWrite(sRabbit.statusLed, HIGH);
	digitalWrite(sRabbit.errorLed, HIGH);
	digitalWrite(sRabbit.button1, LOW);
	digitalWrite(sRabbit.button2, LOW);
	digitalWrite(sRabbit.button3, LOW);
	digitalWrite(sRabbit.button4, LOW);

	Ethernet.begin(sRabbit.mac);
	server.begin();

	digitalWrite(sRabbit.statusLed, LOW);
	digitalWrite(sRabbit.errorLed, LOW);
}

void loop() {	//	Boucle principale
	EthernetClient client = server.available();
	
	if(client){
		// an http request ends with a blank line
		boolean current_line_is_blank = true;

		blink(sRabbit.statusLed, 100, 1);

		while (client.connected()) {
			if(client.available()) {
				char c = client.read();
				// if we've gotten to the end of the line (received a newline
				// character) and the line is blank, the http request has ended,
				// so we can send a reply
				if (inString.length() < 35) {
					inString.concat(c);
				}

				if (c == '\n' && current_line_is_blank) {
					// send a standard http response header
					client.println("HTTP/1.1 200 OK");
					client.println("Content-Type: text/html");
					client.println();
					client.println("<html><style>");
					client.println("html, body { height: 100%; background-color: black; }");
					client.println("table { width: 100%; height: 95%; }");
					client.println("td { text-align: center; width: 50%; height: 45%; font-size:50px; }");
					client.println("input { width: 80%; height: 80%; font-size:50px; }");
					client.println("</style><body><form method=get>");

					client.println("<table><tr>");
					client.println("<td><input type=submit name=rotation value=MODE></td>");
					client.println("<td><input type=submit name=vibration value=MODE></td>");
					client.println("</tr><tr>");
					client.println("<td><input type=submit name=rotation value=POWER></td>");
					client.println("<td><input type=submit name=vibration value=POWER></td>");
					client.println("</tr><tr>");
					client.println("<td style=\"color: red;\">ROTATION</td>");
					client.println("<td style=\"color: green;\">VIBRATION</td>");
					client.println("</table></tr></from></html></body>");

					if(inString.indexOf("rotation=POWER")>0) {
						blink(sRabbit.button1, sRabbit.ulDelay, 1);
					}
					if(inString.indexOf("rotation=MODE")>0) {
						blink(sRabbit.button2, sRabbit.ulDelay, 1);
					}
					if(inString.indexOf("vibration=POWER")>0) {
						blink(sRabbit.button3, sRabbit.ulDelay, 1);
					}
					if(inString.indexOf("vibration=MODE")>0) {
						blink(sRabbit.button4, sRabbit.ulDelay, 1);
					}
					
					break;
				}

				if (c == '\n') {
					// we're starting a new line
					current_line_is_blank = true;
				}
				else if (c != '\r') {
					// we've gotten a character on the current line
					current_line_is_blank = false;
				}
			}
		}
		// give the web browser time to receive the data
//		delay(1);
		inString = "";
		client.stop();
	}

	delay(10);
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
