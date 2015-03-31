#include "GSMDevice.hpp"

using namespace std;

GSMDevice::GSMDevice(){								//Instantiates a GSM device controlled using GPIO 4 (default, software pwr control) and /dev/ttyAMA0 for UART
	GSMDevice(DEFAULT_GPIO,5,DEFAULT_UART);
}

//WARNING: May take up to six seconds
GSMDevice::GSMDevice(int pwrgpio, int statusgpio, string serial){		//Instantiates a GSM device controlled using a GPIO pin (given by GPIO parameter) and a UART port (given by serial){
	this->statusgpio = statusgpio;
	this->pwrgpio = pwrgpio;
	this->serialPort = serial;
	InitSerial();
	InitGSMModule();
	delay(3000);								//Allow some time for the GSM module to connect to the network.
}

//Returns 0 for LOW, 1 for HIGH.
uint16_t GSMDevice::CheckModuleStatus(){
	return digitalRead(this->statusgpio);
}


//Turns the module on.
void GSMDevice::TurnOn(){
	if(!CheckModuleStatus()){
		TogglePWR();
	}
}

//Turns the module off.
void GSMDevice::TurnOff(){
        if(CheckModuleStatus()){
                TogglePWR();
        }
}


void GSMDevice::TogglePWR(){							
	//Toggles the power status of the GSM module. Turns it on if it is off and turns it off if it is on. 
	//WARNING: Takes 2500 miliseconds to complete

	digitalWrite(this->pwrgpio, LOW);					//Spec says that the adafruit FONA expects a 2000ms low pulse
	delay(2000);
	digitalWrite(this->pwrgpio, HIGH);
	delay(500);								//Give the module some time to turn on, ensure that when 
										//function returns the module is operational.
}

bool GSMDevice::Call(string num){						//Establishes a voice connection with a phone number. The function
	char *number = new char[num.length() + 1];				//will return when the voice connection is established. Note that
        strcpy(number, num.c_str());						//having a voice connection does not mean that both parties are
	char send[20];								//ready to exchange data. In fact, the sender should allow for some
										//time before transmitting so that the receiver can answer.

	if(strlen(number)>(sizeof(send)/sizeof(char))-(3+1)){			//If the number does not fit into the buffer, reject it, as 
		if(OPENSTRATOS_GSM_DEBUG_){					//phone numbers (without C.C.) take 9 chars.
                        printf("[!] Number too long. Aborting call\n");
        	}
		return false;
	}
	strncpy(send, "ATD", strlen("ATD"));					//AT command preamble: ATD
  	strncpy(send+3, number, strlen(number));				//Number to dial
  	int length = strlen(send);
	send[length] = (char)0x3b;//ascii for semicolon				//Semicolon ending command
  	send[length+1] = (char)0x0;//terminate string with null byte
	bool success =  SerialSendCompareResponse(send, "OK");			//Modem will reply OK if a voice connection was established.
	if(OPENSTRATOS_GSM_DEBUG_ && !success){
                        printf("[!] ATD+number+; did not reply OK\n");
        }
	return success;
}

bool GSMDevice::AbortCall(){							//Aborts a phone call. No effect if not in a call.
	bool success = SerialSendCompareResponse("ATH","OK");			//Returns true if call aborted or if not in a call.
	if(OPENSTRATOS_GSM_DEBUG_ && !success){					//Expecting the modem to return OK.
                        printf("[!] ATH did not reply OK\n");
        }
	return success;
}

bool GSMDevice::SendSMS(string message, string n){				//Sends a short message to a phone number
	char *destnum = new char[n.length() + 1];
        strcpy(destnum, n.c_str());
	char *messagechar = new char[message.length() + 1];
        strcpy(messagechar, message.c_str());

	if (SerialSendCompareResponse("AT+CMGF=1", "OK")==false){		//Select SMS message format
		if(OPENSTRATOS_GSM_DEBUG_){					//Abort if the modem cannot be set to text mode.
                        printf("[!] AT+CMFG=1 did not reply OK to SMS format selection\n");
                }
		return false;
	}

  	char send[30] = "AT+CMGS=\"";						//SMS command preamble: AT+CMGS="
  	strncpy(send+9 , destnum, 19);						//SMS destination (actual phone number)
  	send[strlen(send)] = (char)0x22;//ascii for " (double quote)		//SMS command termination: "

  	if (!SerialSendCompareResponse(send, "> ")){				//Input prompt should be open and expecting text input
		if(OPENSTRATOS_GSM_DEBUG_){
                        printf("[!] Failed SMS attempt. SIM800L did not provide input prompt\n");
                }
		return false;							//If there is not any input prompt, abort.
	}

  	serialPrintf(fdesc,messagechar);					//Send message
  	serialPrintf(fdesc,"\x1a\r\n");						//SUB control character (ascii 0x1a)+Return
	FlushSerialInput();							//Module will reply with "+CMGS<length>". Not interested in that, so flush.
  	return true;								//Message sent.
}

void GSMDevice::BroadcastGeolocation(string n){					//Tries to perform geolocation and sends coordinates to a phone number via SMS.
	initGPRS();
	uint16_t responseLength = SerialSendRead("AT+CIPGSMLOC=1,1");
	serialin[responseLength+1] = (char)0x0;
	SendSMS(serialin, "699686404");
	tearDownGPRS();
}

void GSMDevice::InitGPRS(){
	SerialSendCompareResponse("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", "OK");	//About to turn on GPRS
	string preamble = "AT+SAPBR=3,1,\"APN\",\"";
	string end = ";";
	string command = preamble + APNNAME + end;
	char *send = new char[command.length() + 1];
        strcpy(send, command.c_str());
	SerialSendCompareResponse(send, "OK");					//Set APN provided.
	SerialSendCompareResponse("AT+SAPBR=1,1","OK");				//Activate GPRS context
	/*SerialSendCompareResponse("AT+SAPBR=2,1","OK");*/			//Not realy necessary
}

void GSMDevice::TearDownGPRS(){
	SerialSendCompareResponse("AT+SAPBR=0,1","OK");
}

bool GSMDevice::CheckGSMModule(){						//Returns true if module is up and running, false if it is not.
	if(SerialSendCompareResponse("AT","OK")){				//Probe the SIM800L, see AT command reference for more info.
		return true;
	}
	return false;
}

bool GSMDevice::InitSerial(){
	char *cstr = new char[this->serialPort.length() + 1];
	strcpy(cstr, this->serialPort.c_str());
	fdesc = serialOpen(cstr, UART_BAUDRATE);
	if(fdesc==-1){								//serialOpen returns a standard linux file descriptor. -1 means failed
		;/*EXCEPTION: NO SERIAL PORT AVAILABLE*/
		if(OPENSTRATOS_GSM_DEBUG_){
			printf("[!] Serial port open error\n");
		}
		return false;
	}
	return true;
}

bool GSMDevice::InitGSMModule(){
	if (wiringPiSetup() == -1){
		/*EXCEPTION: UNABLE TO ACCESS GPIOS*/;
		return false;
	}
	pinMode(this->pwrgpio, OUTPUT);						//Set GPIO as digital output
	digitalWrite(this->pwrgpio, HIGH);						//Set to high so that GSM module stays in standby until TogglePWR is called
	pinMode(this->pwrgpio, INPUT);
	return true;
}

bool GSMDevice::SerialSendCompareResponse(char *send, char *expected){
	SerialSendRead(send);							//Send a string via serial.
  	return (strcmp(serialin, expected) == 0);				//Compare what got placed in serialin with the expected answer
}

uint16_t GSMDevice::SerialSendRead(char *send) {
	FlushSerialInput();
	if(OPENSTRATOS_GSM_DEBUG_){
                        printf("[i] Sending: %s", send);
        }
	serialPrintf(fdesc, send);
	uint16_t readlength = SerialReadLine();
	if(OPENSTRATOS_GSM_DEBUG_){
                        printf("[i] Received: %s\n", serialin);
        }
	return readlength;
}

void GSMDevice::FlushSerialInput() {
	serialFlush(fdesc);
}

uint16_t GSMDevice::SerialReadLine() {
 	uint16_t i = 0;
	while(serialDataAvail(fdesc)&&i<254) {
    		char c =  serialGetchar(fdesc);
    		serialin[i++] = c;
  	}
  	serialin[i] = 0x0;
  	return i;
}
