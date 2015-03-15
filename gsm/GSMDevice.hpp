#ifndef OPENSTRATOS_WIRINGPI_H_
#define OPENSTRATOS_WIRINGPI_H_
#include <wiringPi.h>
#endif

#ifndef OPENSTRATOS_WIRINGSERIAL_H
#define OPENSTRATOS_WIRINGSERIAL_H
#include <wiringSerial.h>
#endif

#ifndef OPENSTRATOS_GSM_INCLUDE_ALL_
#define OPENSTRATOS_GSM_INCLUDE_ALL_
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <stdint.h>
#include <algorithm>
#include <stdio.h>
#endif

#define OPENSTRATOS_GSM_DEBUG_ 0
//0 = disable debug mode
//1 = enable debug mode

#ifndef OPENSTRATOS_GSM_GSMDEVICE_CLASS_
#define OPENSTRATOS_GSM_GSMDEVICE_CLASS_
using namespace std;

class GSMDevice{
	public:
		GSMDevice();						//Instantiates a GSM device controlled using GPIO 4 (default, software pwr control) and /dev/ttyAMA0 for UART
		GSMDevice(int gpio, string serial);			//Instantiates a GSM device controlled using a GPIO pin (given by GPIO parameter) and a UART port (given by serial);
		void TogglePWR();					//Toggles the power status of the GSM module. Turns it on if it is off and turns it off if it is on.
		bool Call(string number);				//Calls a given phone number. Returns true if suceeded, false if failed.
		bool AbortCall();					//Aborts a phone call. Returns true if suceeded, false if failed or not in a call.
		bool SendSMS(string message, string n);			//Sends a short message to a phone number. Returns true if suceeded, false if failed
		void BroadcastGeolocation(string n);			//Tries to perform geolocation and sends coordinates to a phone number via SMS.
		bool CheckGSMModule();					//Returns true if module is up and running, false if it is not
	private:
		uint16_t SerialReadLine();
		uint16_t SerialSendRead(char *);
		void FlushSerialInput();				//Discards all data received from the serial port.
		bool SerialSendCompareResponse(char*, char*);		//Sends a string of data via serial port. Checks that the reply matches with another string.
		bool InitGSMModule();					//Initializes the gsm module. Returns true if successful. False if failed.
		bool InitSerial();					//Initializes the serial interface. Returns true if successful. False if failed.
		int gpionum;						//GPIO pin used for software power control
		string serialPort;					//Serial port identifier. Tipically /dev/ttyAMA0 in Raspbian. 
		int fdesc;						//Standard unix file descriptor associated with the serial port (see serialPort)
		char serialin[255];					//Buffer on which data received via the serial port is placed
};
#endif

#ifndef OPENSTRATOS_GSM_DEFAULT_PARAMETERS_
#define OPENSTRATOS_GSM_DEFAULT_PARAMETERS_
#define DEFAULT_UART "/dev/ttyAMA0"
#define DEFAULT_GPIO 4
#define UART_BAUDRATE 4800
#endif

#ifndef OPENSTRATOS_GSM_GPIO_VALUES_
#define OPENSTRATOS_GSM_GPIO_VALUES_
#define HIGH 1
#define LOW 0
#endif
