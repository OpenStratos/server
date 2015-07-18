#include "gsm/GSM.h"
#include "constants.h"

#include <thread>
#include <string>
#include <sstream>

#include <wiringPi.h>
#include <wiringSerial.h>

using namespace std;
using namespace os;

GSM& GSM::get_instance()
{
	static GSM instance;
	return instance;
}

GSM::~GSM()
{
	this->serial.close();
}

bool GSM::initialize(int pwr_gpio, int status_gpio, const string& serial_URL)
{
	this->pwr_gpio = pwr_gpio;
	this->status_gpio = status_gpio;

	if ( ! this->serial.initialize(serial_URL, 9600))
	{
		return false;
	}

	pinMode(this->pwr_gpio, OUTPUT);
	digitalWrite(this->pwr_gpio, HIGH);
	pinMode(this->pwr_gpio, INPUT);

	this_thread::sleep_for(3s);

	return true;
}

bool GSM::send_SMS(const string& message, const string& number) const
{
	if (this->send_command_read("AT+CMGF=1") != "OK")
	{
		return false;
	}

	stringstream send_command;

	send_command << "AT+CMGS=\"" << number << "\"";

	if (this->send_command_read(send_command.str()) != "> ")
	{
		return false;
	}

	this->serial.send(message+"\x1a"+GSM_ENDL);
	this->serial.flush();

	return true;
}

bool GSM::get_location(double& latitude, double& longitude) const
{
	if ( ! this->init_GPRS())
	{
		return false;
	}
	// TODO
	// OLD:
	// uint16_t responseLength = SerialSendRead("AT+CIPGSMLOC=1,1");
	// serialin[responseLength+1] = (char)0x0;
	if (this->tear_down_GPRS())
	{
		return false;
	}
	return false;
}

bool GSM::get_status() const
{
	return digitalRead(this->status_gpio) == HIGH;
}

bool GSM::is_up() const
{
	return this->send_command_read("AT") == "OK";
}

bool GSM::turn_on() const
{
	if ( ! this->get_status())
	{
		digitalWrite(this->pwr_gpio, LOW);
		this_thread::sleep_for(2s);
		digitalWrite(this->pwr_gpio, HIGH);
		this_thread::sleep_for(500ms);
		return true;
	}
	else
	{
		return false;
	}
}

bool GSM::turn_off() const
{
	if (this->get_status())
	{
		digitalWrite(this->pwr_gpio, LOW);
		this_thread::sleep_for(2s);
		digitalWrite(this->pwr_gpio, HIGH);
		this_thread::sleep_for(500ms);
		return true;
	}
	else
	{
		return false;
	}
}

bool GSM::init_GPRS() const
{
	if (this->send_command_read("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"") != "OK" ||
		this->send_command_read("AT+SAPBR=3,1,\"APN\",\"" + string(GSM_LOC_SERV) + ";") != "OK" ||
		this->send_command_read("AT+SAPBR=1,1") != "OK" ||
		this->send_command_read("AT+SAPBR=2,1") != "OK")
	{
		this->serial.flush();
		return false;
	}
	this->serial.flush();
	return true;
}

bool GSM::tear_down_GPRS() const
{
	return this->send_command_read("AT+SAPBR=0,1") == "OK";
}

const string GSM::send_command_read(const string& command) const
{
	this->serial.flush();
	this->serial.send(command);
	string response = this->serial.read_line();
	this->serial.flush();
	return response;
}
