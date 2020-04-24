/*
 * Spezifikation http://normen.railcommunity.de/RCN-600.pdf
 *
 * Bitte beachten, dass der ESP8266 nur 3.3V vertrÃ¤gt.
 * Das klassische SUSI arbeitet aber mit 5V.
 *
 * Deshalb ist eine Schaltung gemÃ¤ÃŸ Anhang E der Spezifikation notwendig.
 * Die Schaltung ist zumindest in der Spezifikations-Version vom 28.08.2017
 * falsch!
 *
 */
#ifdef ESP8266

#include <Arduino.h>
#include "Consts.h"
#include "ActionSUSIGeneration.h"
#include "Logger.h"
#include "Utils.h"
#include "GPIO.h"
#include "SPINonBlocking.h"

ActionSUSIGeneration::ActionSUSIGeneration(int locoaddr) {
	LOCO_ADR = locoaddr;
	Logger::getInstance()->addToLog(LogLevel::INFO, "Starting Susi Generator");
	Logger::getInstance()->addToLog(LogLevel::INFO, "SUSI-Output:" + GPIOobj.gpio2string(mySPI.getUsedPin())
									+ " Loko-Adresse: " + String(LOCO_ADR)
	);

	// TODO Negieren oder nicht negieren
	SPISettings spiS = SPISettings(17241, LSBFIRST, my_SPI_MODE3, true);
	mySPI.begin(spiS, "SUSI");
	mySPI.beginTransaction(spiS);

	r = new requestInfo();
	r->art = requestInfo::ART::LOCO;
	r->id = LOCO_ADR;
}

ActionSUSIGeneration::~ActionSUSIGeneration() {
	mySPI.end();
}


void ActionSUSIGeneration::fillSpiBuffer() {
	unsigned char speed = 0;
	speed = SPEED_STATE & 127;
	if (SPEED_STATE == Consts::SPEED_EMERGENCY || SPEED_STATE == Consts::SPEED_STOP) {
		speed = 0;
	}
	if (DIR_STATE == Consts::SPEED_FORWARD) {
		speed += 128; // Set High Bit
	}

	SPIBufUsed = 0;
	// IST Lok Fahrstufe
	SPIBuf[SPIBufUsed++] = B00100100;
	SPIBuf[SPIBufUsed++] = speed;

	// SOLL Lok Fahrstufe
	SPIBuf[SPIBufUsed++] = B00100101;
	SPIBuf[SPIBufUsed++] = speed;

	// FG1 --   00 00 00 F0 --  F4 F3 F2 F1
	SPIBuf[SPIBufUsed++] = B01100000;
	SPIBuf[SPIBufUsed++] = ((FUNC_STATE >> 1) & 15) | ((FUNC_STATE & 1) << 4);

	// FG2  F5 bis F12
	SPIBuf[SPIBufUsed++] = B01100001;
	SPIBuf[SPIBufUsed++] = ((FUNC_STATE >> 5) & 0xFF);

	// FG2  F13 bis F20
	SPIBuf[SPIBufUsed++] = B01100010;
	SPIBuf[SPIBufUsed++] = ((FUNC_STATE >> 13) & 0xFF);

	// FG2  F21 bis F28
	SPIBuf[SPIBufUsed++] = B01100011;
	SPIBuf[SPIBufUsed++] = ((FUNC_STATE >> 21) & 0xFF);
}


void ActionSUSIGeneration::invertBuffer() {
	for (int i = 0; i < SPIBufUsed; i++) {
		SPIBuf[i] = ~SPIBuf[i] & 0XFF;
	}
}
/**
 * Wenn der SPI-Bus frei ist, neues Paket erzeugen und verschicken
 */
int ActionSUSIGeneration::loop() {
	if (mySPI.busy()) {
		return 0;
	}
	fillSpiBuffer();
	invertBuffer();
	send();
	return 30;
}



void ActionSUSIGeneration::DCCFunc(int id, unsigned long int newvalue, int source) {
	if (id == LOCO_ADR || id == Consts::LOCID_ALL) {
		FUNC_STATE = newvalue;
	}
}


void ActionSUSIGeneration::DCCSpeed(int id, int speed, int direction, int SpeedSteps, int source) {
	if (id == LOCO_ADR || id == Consts::LOCID_ALL) {
		if (speed == Consts::SPEED_EMERGENCY || speed == Consts::SPEED_STOP) {
			speed = 0;
		} else if (SpeedSteps == 29) {
			speed = speed * 127.0f / SpeedSteps;
		}
		DIR_STATE = (direction == Consts::SPEED_FORWARD);
		SPEED_STATE = speed;
	}
}

void ActionSUSIGeneration::send() {
	if (SPIBufUsed == 0) {
		return;
	}
	mySPI.send(SPIBuf, SPIBufUsed);
	SPIBufUsed = 0;
}
#endif

