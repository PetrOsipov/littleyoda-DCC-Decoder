/*
 * ActionPWMOutput.h
 *
 *  Created on: 09.12.2016
 *      Author: sven
 */

#ifndef ACTIONPWMOUTPUT_H_
#define ACTIONPWMOUTPUT_H_

#include <Arduino.h>

#include "ISettings.h"

class ActionPWMOutput: public ISettings {
public:
	ActionPWMOutput();
	virtual ~ActionPWMOutput();
	virtual String getHTMLController(String urlprefix);
	virtual void getHTMLConfig(String urlprefix, Controller* c);
	void setPwmValues(uint8_t* arr);
	uint8_t getValue(uint8_t pos);
	virtual void setFreq(uint32_t value);

private:
	uint8_t* arr;
};

#endif /* ACTIONPWMOUTPUT_H_ */

