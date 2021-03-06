#include "gsm/GSM.h"
#include "constants.h"

#include <thread>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <chrono>

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
	if (this->serial->is_open())
	{
		this->logger->log("Closing serial interface...");
		this->serial->close();
		this->logger->log("Serial interface closed.");
		this->logger->log("Deallocating serial...");
		delete this->serial;
		this->logger->log("Serial deallocated");

		this->logger->log("Deallocating command logger...");
		delete this->command_logger;
	}

	if (this->is_on())
	{
		this->logger->log("Shutting down...");
		this->turn_off();
		this->logger->log("Shut down finished.");
	}
	delete this->logger;
}

bool GSM::initialize()
{
	this->occupied = false;

	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm * now = gmtime(&timer.tv_sec);

	this->logger = new Logger("data/logs/GSM/GSM."+ to_string(now->tm_year+1900) +"-"+ to_string(now->tm_mon+1) +"-"+
		to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+ to_string(now->tm_min) +"-"+
		to_string(now->tm_sec) +".log", "GSM");

	this->command_logger = new Logger("data/logs/GSM/GSMCommands."+ to_string(now->tm_year+1900) +"-"+
		to_string(now->tm_mon+1) +"-"+ to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+
		to_string(now->tm_min) +"-"+ to_string(now->tm_sec) +".log", "GSMCommand");

	pinMode(GSM_PWR_GPIO, OUTPUT);
	digitalWrite(GSM_PWR_GPIO, HIGH);
	pinMode(GSM_STATUS_GPIO, INPUT);

	if (this->is_on())
	{
		this->logger->log("Module is on, rebooting for stability.");
		this->turn_off();
		this->logger->log("Module off. Sleeping 3 seconds before turning it on...");
		this_thread::sleep_for(3s);
	}

	this->logger->log("Turning module on...");
	this->turn_on();
	if ( ! this->is_on())
	{
		this->logger->log("Error: The module is still off. Finishing initialization.");
		return false;
	}
	this->logger->log("Sleeping 3 seconds to let it turn completely on...");
	this_thread::sleep_for(3s);

	this->occupied = true;
	this->logger->log("Starting serial connection...");
	#ifdef DEBUG
		this->serial = new Serial(GSM_UART, GSM_BAUDRATE, "GSM");
	#else
		this->serial = new Serial(GSM_UART, GSM_BAUDRATE);
	#endif
	if ( ! this->serial->is_open())
	{
		this->logger->log("GSM serial error.");
		this->occupied = false;
		return false;
	}
	this->logger->log("Serial connection started.");

	this->logger->log("Deleting possible serial characters...");
	this->serial->flush();

	this->logger->log("Checking OK initialization (3 times)...");
	if (this->send_command_read("AT") != "OK")
	{
		this->logger->log("Not initialized.");
	}
	this_thread::sleep_for(100ms);

	if (this->send_command_read("AT") != "OK")
	{
		this->logger->log("Not initialized.");
	}
	this_thread::sleep_for(100ms);

	if (this->send_command_read("AT") != "OK")
	{
		this->logger->log("Error on initialization.");
		this->occupied = false;
		return false;
	}
	this_thread::sleep_for(100ms);
	this->logger->log("Initialization OK.");
	this->occupied = false;

	return true;
}

bool GSM::send_SMS(const string& message, const string& number)
{
	while (this->occupied)
	{
		this_thread::sleep_for(10ms);
	}
	this->occupied = true;

	this->logger->log("Sending SMS: \""+message+"\" ("+ to_string(message.length()) +" characters) to number "+number+".");
	if (message.length() > 160)
	{
		this->logger->log("Error: SMS has more than 160 characters");
	}
	else
	{
	#ifndef NO_SMS
		if (this->send_command_read("AT+CMGF=1") != "OK")
		{
			this->logger->log("Error sending SMS on 'AT+CMGD=1' response.");
			this->occupied = false;
			return false;
		}

		if (this->send_command_read("AT+CMGS=\""+number+"\"") != "> ")
		{
			this->logger->log("Error sending SMS on 'AT+CMGS' response.");
			this->occupied = false;
			return false;
		}

		this->serial->println(message);
		this->command_logger->log("Sent: '"+message+"'");

		for (int i = 0; i <= count(message.begin(), message.end(), '\n'); ++i)
		{
			// Eat message echo
			this->command_logger->log(
				"Received: '"+this->serial->read_line()+"'"
			);
		}

		this->serial->println();
		this->serial->read_line(); // Eat prompt
		this->serial->write_byte(0x1A);
		this->serial->read_line(60); // Eat prompt (timeout 60 seconds)

		// Read +CMGS response
		string response = this->serial->read_line(10);
		this->command_logger->log("Received: '"+response+"'");
		if (response.find("+CMGS") == string::npos)
		{
			this->logger->log("Error sending SMS. Could not read '+CMGS'.");
			this->occupied = false;
			return false;
		}
		this->serial->read_line(); // Eat new line

		// Read OK (timeout 10 seconds)
		response = this->serial->read_line(10);
		this->command_logger->log("Received: '"+response+"'");
		if (response != "OK")
		{
			this->logger->log("Error sending SMS. Could not read 'OK'.");
			this->occupied = false;
			return false;
		}
	#else
		this_thread::sleep_for(5s);
	#endif
	}
	this->occupied = false;

	this->logger->log("SMS sent.");
	return true;
}

bool GSM::get_location(double& latitude, double& longitude)
{
	while (this->occupied)
	{
		this_thread::sleep_for(10ms);
	}
	this->occupied = true;

	if (this->send_command_read("AT+CMGF=1") != "OK")
	{
		this->logger->log("Error getting location on 'AT+CMGD=1' response.");
		this->occupied = false;
		return false;
	}

	if (this->send_command_read("AT+CGATT=1") != "OK")
	{
		this->logger->log("Error getting location on 'AT+CGATT=1' response.");
		if (this->send_command_read("AT+SAPBR=0,1") != "OK")
		{
			this->logger->log("Error turning GPRS down.");
		}
		else
		{
			this->logger->log("GPRS off.");
		}

		this->occupied = false;
		return false;
	}

	if (this->send_command_read("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"") != "OK")
	{
		this->logger->log("Error getting location on 'AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"' response.");
		if (this->send_command_read("AT+SAPBR=0,1") != "OK")
		{
			this->logger->log("Error turning GPRS down.");
		}
		else
		{
			this->logger->log("GPRS off.");
		}

		this->occupied = false;
		return false;
	}

	if (this->send_command_read("AT+SAPBR=3,1,\"APN\",\""+string(GSM_LOC_SERV)+"\"") != "OK")
	{
		this->logger->log("Error getting location on 'AT+SAPBR=3,1,\"APN\",\""+string(GSM_LOC_SERV)+"\"' response.");
		if (this->send_command_read("AT+SAPBR=0,1") != "OK")
		{
			this->logger->log("Error turning GPRS down.");
		}
		else
		{
			this->logger->log("GPRS off.");
		}

		this->occupied = false;
		return false;
	}

	this->serial->println("AT+SAPBR=1,1");
	this->command_logger->log("Sent: 'AT+SAPBR=1,1'");
	this->serial->read_line(2); // Eat message echo

	string response = this->serial->read_line();
	this->command_logger->log("Received: '"+ response +"'");

	if (response != "OK")
	{
		this->logger->log("Error getting location on 'AT+SAPBR=1,1' response.");
		if (this->send_command_read("AT+SAPBR=0,1") != "OK")
		{
			this->logger->log("Error turning GPRS down.");
		}
		else
		{
			this->logger->log("GPRS off.");
		}

		this->occupied = false;
		return false;
	}

	this->serial->println("AT+CIPGSMLOC=1,1");
	this->serial->read_line(10); // Eat message echo
	response = this->serial->read_line();
	this->command_logger->log("Received: '"+ response +"'");

	stringstream ss(response);
	string data;
	vector<string> s_data;

	// We put all fields in a vector
	while(getline(ss, data, ','))
	{
		s_data.push_back(data);
	}

	latitude = stod(s_data[2]);
	longitude = stod(s_data[1]);

	this->serial->read_line(); // Eat new line
	response = this->serial->read_line();
	this->command_logger->log("Received: '"+ response +"'");
	if (response != "OK")
	{
		this->logger->log("Error getting location on 'AT+CIPGSMLOC=1,1' response.");
		if (this->send_command_read("AT+SAPBR=0,1") != "OK")
		{
			this->logger->log("Error turning GPRS down.");
		}
		else
		{
			this->logger->log("GPRS off.");
		}

		this->occupied = false;
		return false;
	}

	this->serial->println("AT+SAPBR=0,1");
	this->serial->read_line(2); // Eat message echo
	response = this->serial->read_line();
	this->command_logger->log("Received: '"+ response +"'");
	if (response != "OK")
	{
		this->logger->log("Error turning GPRS down.");
	}
	else
	{
		this->logger->log("GPRS off.");
	}

	this->occupied = false;
	return true;
}

bool GSM::is_on() const
{
	return digitalRead(GSM_STATUS_GPIO) == HIGH;
}

bool GSM::get_battery_status(double& main_bat_percentage, double& gsm_bat_percentage)
{
	while (this->occupied)
	{
		this_thread::sleep_for(10ms);
	}
	this->occupied = true;

	this->logger->log("Checking Battery status.");
	if (this->is_on())
	{
		string gsm_response = this->send_command_read("AT+CBC");
		this->serial->read_line(); // Eat new line
		this->serial->read_line(); // Eat OK
		string adc_response = this->send_command_read("AT+CADC?");
		this->serial->read_line(); // Eat new line
		this->serial->read_line(); // Eat OK

		chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
		while (adc_response != "" &&
				adc_response.substr(0, 6) != "+CADC:" &&
				(chrono::high_resolution_clock::now() - start) < 1s)
		{
			adc_response = this->serial->read_line();
		}

		if (gsm_response.length() >= 5 &&
			gsm_response.substr(0, 5) == "+CBC:" &&
			adc_response.length() >= 6 &&
			adc_response.substr(0, 6) == "+CADC:")
		{
			stringstream gsm_ss(gsm_response);
			string data;
			vector<string> gsm_data;

			while(getline(gsm_ss, data, ','))
			{
				gsm_data.push_back(data);
			}

			if (gsm_data.size() >= 3)
			{
				try
				{
					int gsm_bat_voltage = stoi(gsm_data[2]);
					int main_bat_voltage = stoi(adc_response.substr(9, 4));
					gsm_bat_percentage = (gsm_bat_voltage/1000.0-BAT_GSM_MIN)/(BAT_GSM_MAX-BAT_GSM_MIN);
					main_bat_percentage = (main_bat_voltage/1000.0-BAT_MAIN_MIN)/(BAT_MAIN_MAX-BAT_MAIN_MIN);
				}
				catch (const invalid_argument& ia)
				{
					this->occupied = false;
					return false;
				}

				this->occupied = false;
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		this->logger->log("Error: module is off.");
	}
	this->occupied = false;
	return false;
}

bool GSM::has_connectivity()
{
	while (this->occupied)
	{
		this_thread::sleep_for(10ms);
	}
	this->occupied = true;

	string response = this->send_command_read("AT+CREG?");
	this->serial->read_line(); // Eat new line
	this->serial->read_line(); // Eat OK
	this->occupied = false;

	return response == "+CREG: 0,1" || response == "+CREG: 0,5";
}

bool GSM::turn_on() const
{
	if ( ! this->is_on())
	{
		this->logger->log("Turning GSM on...");

		digitalWrite(GSM_PWR_GPIO, LOW);
		this_thread::sleep_for(2s);
		digitalWrite(GSM_PWR_GPIO, HIGH);

		this_thread::sleep_for(3s);

		this->logger->log("GSM on.");
		return true;
	}
	else
	{
		this->logger->log("Error: Turning on GSM but GSM already on.");
		return false;
	}
}

bool GSM::turn_off() const
{
	if (this->is_on())
	{
		this->logger->log("Turning GSM off...");

		digitalWrite(GSM_PWR_GPIO, LOW);
		this_thread::sleep_for(2s);
		digitalWrite(GSM_PWR_GPIO, HIGH);

		this_thread::sleep_for(3s);

		this->logger->log("GSM off.");
		return true;
	}
	else
	{
		this->logger->log("Error: Turning off GSM but GSM already off.");
		return false;
	}
}

const string GSM::send_command_read(const string& command) const
{
	this->command_logger->log("Sent: '"+command+"'");
	this->serial->flush();
	this->serial->println(command);
	string response = this->serial->read_line();
	if ( ! response.empty())
	{
		// Trimming
		string ltrim = response.erase(0, response.find_first_not_of("\r\n\t"));
		response = ltrim.erase(ltrim.find_last_not_of("\r\n\t")+1);

		if (response == command) // Sent command
		{
			response = this->serial->read_line();
			// Trimming
			string ltrim = response.erase(0, response.find_first_not_of("\r\n\t"));
			response = ltrim.erase(ltrim.find_last_not_of("\r\n\t")+1);
		}
	}

	this->command_logger->log("Received: '"+response+"'");
	return response;
}
