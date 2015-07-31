#include "gsm/GSM.h"
#include "constants.h"

#include <thread>
#include <string>
#include <sstream>

#include <sys/time.h>

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
	if (this->serial.is_open())
	{
		this->logger->log("Closing serial interface...");
		this->serial.close();
		this->logger->log("Serial interface closed.");

		this->logger->log("Deallocating command logger...");
		delete this->command_logger;
	}
	this->logger->log("Shutting down...");
	this->turn_off();
	this->logger->log("Shut down finished.");
	delete this->logger;
}

bool GSM::initialize(int pwr_gpio, int status_gpio, const string& serial_URL)
{
	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm * now = gmtime(&timer.tv_sec);

	this->logger = new Logger("data/logs/GSM/GSM."+ to_string(now->tm_year+1900) +"-"+ to_string(now->tm_mon) +"-"+
		to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+ to_string(now->tm_min) +"-"+
		to_string(now->tm_sec) +".log", "GSM");

	this->pwr_gpio = pwr_gpio;
	this->status_gpio = status_gpio;

	this->logger->log("Starting serial connection...");
	if ( ! this->serial.initialize(serial_URL, GSM_BAUDRATE))
	{
		this->logger->log("GSM serial error.");
		return false;
	}
	this->logger->log("Serial connection started.");

	this->command_logger = new Logger("data/logs/GSM/GSMCommands."+ to_string(now->tm_year+1900) +"-"+
		to_string(now->tm_mon) +"-"+ to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+
		to_string(now->tm_min) +"-"+ to_string(now->tm_sec) +".log", "GSMCommand");

	pinMode(this->pwr_gpio, OUTPUT);
	digitalWrite(this->pwr_gpio, HIGH);
	pinMode(this->status_gpio, INPUT);
	return true;
}

bool GSM::send_SMS(const string& message, const string& number) const
{
	this->logger->log("Sending SMS: \""+message+"\" to number "+number+".");

	if (this->send_command_read("AT+CMGF=1") != "OK")
	{
		this->logger->log("Error sending SMS");
		this->logger->log("Primer condicional");
		return false;
	}

	stringstream send_command;

	send_command << "AT+CMGS=\"" << number << "\"";

	if ( ! this->send_command_read_only(send_command.str(), ">"))
	{
		this->logger->log("Error sending SMS.");
		this->logger->log("Segundo condicional");
		return false;
	}

	this->send_command(message+"\x1a\r\n");
	this->serial.flush();

	this->logger->log("SMS sent.");
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
	return this->send_command_read_only("AT", "OK");
}

bool GSM::has_connectivity() const
{
	   string response = this->send_command_read("AT+CREG?");
	   return response == "+CREG: 0,1" || response == "+CREG: 0,5";
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
	this->command_logger->log("Sent: '"+command+"'");
	this->serial.flush();
	this->serial.send(command);
	// Sent command
	this->serial.read_line();
	string response = this->serial.read_line();
	this->command_logger->log("Received: '"+response+"'");
	this->serial.flush();
	return response;
}

bool GSM::send_command_read_only(const string& command, const string& only) const
{
	this->command_logger->log("Sent: '"+command+"'");
	this->serial.flush();
	this->serial.send(command);
	// Sent command
	this->serial.read_line();
	bool response = this->serial.read_only(only);
	if (response)
	{
		this->command_logger->log("Received: '"+only+"'");
	}
	this->serial.flush();
	return response;
}

void GSM::send_command(const string& command) const
{
	this->command_logger->log("Sent: '"+command+"'");
	this->serial.flush();
	this->serial.send(command);
	this_thread::sleep_for(25ms);
	this->serial.flush();
}
