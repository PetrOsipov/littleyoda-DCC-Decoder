/*
 * ConnectorONOFF.cpp
 *
 *  Created on: 19.07.2017
 *      Author: sven
 */

#include "ConnectorONOFF.h"

ConnectorONOFF::ConnectorONOFF(ISettings* a, int locoaddr, int f) {
  Serial.println("Connector on off - create ");
	addAction(a);
	addr = locoaddr;
	funcbit = f;

	r = new requestInfo();
	r->art = requestInfo::ART::LOCO;
	r->id = addr;
	setModulName("ON/OFF");
	setConfigDescription(String(locoaddr) + "/F" + String(f));

}

ConnectorONOFF::~ConnectorONOFF() {
}

void ConnectorONOFF::DCCFunc(int id, int bit, int newvalue, int source) {
  Serial.println("Connector on off - dcc data ");
	if (id == this->addr && bit == this->funcbit) {
		send("onoff", String(newvalue));
	}
}


