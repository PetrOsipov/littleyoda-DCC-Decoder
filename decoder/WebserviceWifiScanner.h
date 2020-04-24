/*
 * WebserviceWifiScanner.h
 *
 *  Created on: 21.08.2016
 *      Author: sven
 */

#ifndef WEBSERVICEWIFISCANNER_H_
#define WEBSERVICEWIFISCANNER_H_

#include <WiFiClient.h>
#include "WebserviceBase.h"

class WebserviceWifiScanner: public WebserviceBase {
public:
	WebserviceWifiScanner();
	virtual ~WebserviceWifiScanner();
	virtual char const* getUri();
	virtual void run();
	virtual String  getLinkText();

private:
	int dbm2qual(int dbm);
};

#endif /* WEBSERVICEWIFISCANNER_H_ */

