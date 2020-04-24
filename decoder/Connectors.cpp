/*
 * Connectors.cpp
 *
 *  Created on: 18.07.2017
 *      Author: sven
 */

#include "Connectors.h"

#include "Connectors.h"
#include <Arduino.h>

Connectors::Connectors() {
}

Connectors::~Connectors() {
}




void Connectors::addAction(ISettings* a) {
    actions.add(a);
}



void Connectors::send(String key, String value) {
    Serial.println("Sending " + key + " " + value);
    for (int idx = 0; idx < actions.size(); idx++) {
        Serial.println("Sending " + key + " " + value + " to " + actions.get(idx)->getName());
        actions.get(idx)->setSettings(key, value);
    }
}

LinkedList<ISettings*>* Connectors::getActions() {
    return &actions;
}

String Connectors::createDebugDiagramm(String parent) {
    String out = getName() + "[label =\" " + getModulName() + "\\n" + getConfigDescription() + "\"];\r\n";
    for (int idx = 0; idx < actions.size(); idx++) {
        out = out + actions.get(idx)->createDebugDiagramm(getName());
    }
    return out;
}

