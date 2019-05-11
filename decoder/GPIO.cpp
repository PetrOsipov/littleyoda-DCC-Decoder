/*
 * GPIO.cppgpg
 *
 *  Created on: 15.07.2017
 *      Author: sven
 */

#include "GPIO.h"

GPIOClass::GPIOClass() {
	mcps = new LinkedList<Adafruit_MCP23017*>();
	data = new DataContainerSimpleList<String, int16_t>(16* 6 + 12, "", -1);
	valueinputpins = new DataContainerSimpleList<int16_t, int16_t>(16* 6 + 12, -1, -1);
	pinusage = new DataContainer<int16_t, String>(-1, "");
	cacheEnabled = false;
	cachedValue = NULL;
#ifdef ARDUINO_ESP8266_ESP01
	add("D0", 16);
	add("D2", 4);
	add("D3", 0);
	add("D4", 2);
	add("D5", 14);
	add("D7", 13);
#endif
#ifdef ARDUINO_ESP8266_WEMOS_D1MINI
	add("D0", D0);
	add("D1", D1);
	add("D2", D2);
	add("D3", D3);
	add("D4", D4);
	add("D5", D5);
	add("D6", D6);
	add("D7", D7);
	add("D8", D8);
#endif
#ifdef ARDUINO_ESP8266_NODEMCU
	add("D0", D0);
	add("D1", D1);
	add("D2", D2);
	add("D3", D3);
	add("D4", D4);
	add("D5", D5);
	add("D6", D6);
	add("D7", D7);
	add("D8", D8);
	add("D9", D9);
	add("D10", D10);
#endif
	add("DISABLE", Consts::DISABLE);
}

/**
 * "D4", 14
 */
void GPIOClass::add(String s, int pinNumber) {
	data->put(s, pinNumber);
}

GPIOClass::~GPIOClass() {
}

/**
 * 14 => "D4"
 */
String GPIOClass::gpio2string(int gpio) {
	if (!data->containsValue(gpio)) {
		Logger::getInstance()->addToLog("Unbekannter GPIO: " + String(gpio));
		return "Pin " + String(gpio);
	}
	return data->getKeyByValue(gpio);
}


/**
 * "D4" => 14
 */
int GPIOClass::string2gpio(const char* pin) {
	if (pin == NULL) {
		Logger::log("PIN fehlt (null in string2gpio)");
		return Consts::DISABLE;
	}
	String s = String(pin);
	if (!data->containsKey(s)) {
		Logger::getInstance()->addToLog("Unbekannter Pin in Config: " + s);
		return Consts::DISABLE;
	}
	return data->getValueByKey(s);
}

int GPIOClass::string2gpio(String pin) {
	if (pin == NULL) {
		Logger::log("PIN fehlt (null in string2gpio)");
		return Consts::DISABLE;
	}
	if (!data->containsKey(pin)) {
		Logger::getInstance()->addToLog("Unbekannter Pin in Config: " + pin);
		return Consts::DISABLE;
	}
	return data->getValueByKey(pin);
}

void GPIOClass::pinMode(Pin* pin, uint8_t mode, String usage) {
	pinMode(pin->getPin(), mode, usage);
}

void GPIOClass::pinMode(uint16_t pin, uint8_t mode, String usage) {
	if (pin == Consts::DISABLE) {
		Logger::getInstance()->addToLog(
				"Accessing Disabled Pin (pinMode): " + String(pin));
		return;
	}
	if (pin >= 100) {
		if (mode != INPUT && mode != OUTPUT && mode != INPUT_PULLUP ) {
			Logger::getInstance()->addToLog("Unsupported PinMode: " + String(mode) + " for pin " + String(pin));
			return;
		}
		int mcpIdx = (pin / 100) - 1;
		int realPin = pin % 100;
		Adafruit_MCP23017* mcp = mcps->get(mcpIdx);
		if (mode == INPUT_PULLUP) {
			mcp->pinMode(realPin, INPUT);
			mcp->pullUp(realPin, HIGH);
		} else {
			mcp->pinMode(realPin, mode);
		}
	} else {
		::pinMode(pin, mode);
	}
	if (mode == INPUT || mode == INPUT_PULLUP ) {
		int v = digitalRead(pin);
		valueinputpins->put(pin, v);
	}
	addUsage(pin, usage);
}

void GPIOClass::digitalWrite(Pin* pin, uint8_t val) {
	if (pin->isInvert()) {
		if (val == 0) {
			val = 1;
		} else if (val == 1) {
			val = 0;
		}
	}
	digitalWrite(pin->getPin(), val);
}

void GPIOClass::digitalWrite(uint16_t pin, uint8_t val) {
	if (pin == Consts::DISABLE) {
		Logger::getInstance()->addToLog(
				"Accessing Disabled Pin (pinMode): " + String(pin));
		return;
	}
	if (pin >= 100) {
		int mcpIdx = (pin / 100) - 1;
		int realPin = pin % 100;
		return mcps->get(mcpIdx)->digitalWrite(realPin, val);
	} else {
		::digitalWrite(pin, val);
	}
}

void GPIOClass::analogWrite(uint16_t pin, int val) {
	if (pin == Consts::DISABLE) {
		Logger::getInstance()->addToLog(
				"Accessing Disabled Pin (pinMode): " + String(pin));
		return;
	}
	if (pin >= 100) {
		Logger::getInstance()->addToLog(
				"Analog Write not possible for Pin: " + String(pin));
		return;
	}
	#ifdef ESP8266
		::analogWrite(pin, val);
	#elif ESP32
		// Hack
	#endif

}

void GPIOClass::analogWriteFreq(uint32_t freq) {
	#ifdef ESP8266
		::analogWriteFreq(freq);
	#elif ESP32
		// Hack
	#endif
}

/**
 * Fügt die sprechenden PIN Bezeichner (ExA0 bis ExB7) für die MCP23017 hinzu
 */

void GPIOClass::addMCP23017(uint8_t addr) {
	Adafruit_MCP23017* m = new Adafruit_MCP23017();
	int idx = mcps->size();
	int offset = (idx + 1) * 100;
	m->begin(addr);
	for (int i=0 ; i < 8; i++) {
		add("E" + String(idx) + "A" + String(i), offset + i);
		add("E" + String(idx) + "B" + String(i), offset + 8 + i);
	}
	mcps->add(m);
}

GPIOClass GPIOobj;

int GPIOClass::digitalRead(uint16_t pin) {
	if (pin >= 100) {
		int mcpIdx = (pin / 100) - 1;
		int realPin = pin % 100;
		if (cacheEnabled) {
			uint16_t t = cachedValue[mcpIdx];
			t = (t >> realPin) & 1;
			return (int) t;
		} else {
			return mcps->get(mcpIdx)->digitalRead(realPin);
		}
	} else {
		return ::digitalRead(pin);
	}
}

int GPIOClass::digitalRead(Pin* pin) {
	int v = digitalRead(pin->getPin());
	if (pin->isInvert()) {
		if (v == 0) {
			v = 1;
		} else if (v == 1) {
			v = 0;
		}
	}
	return v;
}

void GPIOClass::setController(Controller* c) {
	controller = c;
}

void GPIOClass::cache(bool b) {
	cacheEnabled = b;
	if (b) {
		int size = mcps->size();
		if (cachedValue == NULL) {
			cachedValue = new uint16_t[size];
		}
		for (int i = 0; i < size; i++) {
			cachedValue[i] = mcps->get(i)->readGPIOAB();
		}
	}

}

void GPIOClass::addUsage(uint16_t pin, String usage) {
	return;
	Serial.println("Adding " + usage + " to " + String(pin));
	String value = usage;
	if (pinusage->containsKey(pin)) {
		String oldvalue = pinusage->getValueByKey(pin);
		value = oldvalue + "; " + value;
		pinusage->removeByKey(pin);
	}
	pinusage->put(pin, value);
}

String GPIOClass::getUsage(String sep) {
//TODO	String out = data->toString(sep) + sep + pinusage->toString(sep) + sep;
	String out = "";

//	TODO for (std::map<int, int>::iterator i = valueinputpins.begin (); i != valueinputpins.end (); i++) {
//		int pin = (*i).first;
//		int oldval = (*i).second;
//		out += "Pin: " + String(pin) + " Status: " + String(oldval) + sep;
//	}
	return out;
}



int GPIOClass::loop() {
	cache(true);
	for (int i = 0; i < valueinputpins->used(); i++) {
		int pin = valueinputpins->getKey(i);
		int oldval = valueinputpins->getValue(i);
		int val = digitalRead(pin);
		if (val != oldval) {
			controller->notifyGPIOChange(pin, val);
			valueinputpins->put(pin, val);
		}
	}
	cache(false);
	return 30;
}