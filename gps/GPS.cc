#include "gps/GPS.h"
#include "constants.h"

#include <functional>
#include <vector>
#include <sstream>
#include <string>
#include <regex>
#include <thread>

#include <sys/time.h>

#include <wiringPi.h>

#include "constants.h"
#include "serial/Serial.h"
#include "logger/Logger.h"


using namespace std;
using namespace os;

GPS& GPS::get_instance()
{
	static GPS instance;
	return instance;
}

GPS::~GPS()
{
	if ( ! this->stopped)
	{
		this->logger->log("Stopping GPS thread...");
		this->should_stop = true;
		while ( ! this->stopped) this_thread::sleep_for(1ms);
		this->logger->log("GPS thread stopped");
	}

	if (this->serial->is_open())
	{
		this->logger->log("Closing serial interface...");
		this->serial->close();
		this->logger->log("Serial interface closed.");
		this->logger->log("Deallocating serial...");
		delete this->serial;
		this->logger->log("Serial deallocated");

		this->logger->log("Deallocating frame logger...");
		delete this->frame_logger;
		this->logger->log("Frame logger deallocated");
	}
	this->logger->log("Turning off GPS...");
	this->turn_off();
	this->logger->log("GPS off.");
	delete this->logger;
}

bool GPS::initialize()
{
	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm * now = gmtime(&timer.tv_sec);

	this->logger = new Logger("data/logs/GPS/GPS."+ to_string(now->tm_year+1900) +"-"+ to_string(now->tm_mon) +"-"+
		to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+ to_string(now->tm_min) +"-"+
		to_string(now->tm_sec) +".log", "GPS");

	this->frame_logger = new Logger("data/logs/GPS/GPSFrames."+ to_string(now->tm_year+1900) +"-"+
		to_string(now->tm_mon) +"-"+ to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+
		to_string(now->tm_min) +"-"+ to_string(now->tm_sec) +".log", "GPSFrame");

	this->should_stop = false;
	this->stopped = true;

	#ifndef OS_TESTING
		pinMode(GPS_ENABLE_GPIO, OUTPUT);

		this->logger->log("Turning GPS on...");
		this->turn_on();
		this->logger->log("GPS on.");
	#endif

	this->logger->log("Starting serial connection...");
	this->serial = new Serial(GPS_UART, GPS_BAUDRATE, "GPS");
	if ( ! this->serial->is_open()) {
		this->logger->log("GPS serial error.");
		return false;
	}
	this->logger->log("Serial connection started.");

	this->logger->log("Starting GPS frame thread...");
	thread t(&GPS::gps_thread, this);
	t.detach();
	this->logger->log("GPS frame thread running.");

	#ifndef OS_TESTING
		this->logger->log("Sending configuration frames...");
		this->serial->println("$PMTK220,100*2F");
		this->frame_logger->log("Sent: $PMTK220,100*2F");
		this->serial->println("$PMTK314,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29");
		this->frame_logger->log("Sent: $PMTK314,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29");
		this->logger->log("Configuration frames sent.");
	#endif

	return true;
}

bool GPS::turn_on() const
{
	if (digitalRead(GPS_ENABLE_GPIO) == LOW)
	{
		digitalWrite(GPS_ENABLE_GPIO, HIGH);
		return true;
	}
	else
	{
		return false;
	}
}

bool GPS::turn_off() const
{
	if (digitalRead(GPS_ENABLE_GPIO) == HIGH)
	{
		digitalWrite(GPS_ENABLE_GPIO, LOW);
		return true;
	}
	else
	{
		return false;
	}
}

void GPS::gps_thread()
{
	this->stopped = false;
	string response;

	while( ! this->should_stop)
	{
		#ifndef OS_TESTING
			int available = this->serial->available();

			if (available > 0)
			{
				for (int i = 0; i < available; i++)
				{
					char c = this->serial->read_char();
					response += c;
					if (response[response.length()-2] == '\r' && c == '\n')
					{
						response = response.substr(0, response.length()-2);

						if (response.at(0) == '$')
						{
							this->parse(response);
						}
						response = "";
						this_thread::sleep_for(50ms);
					}
				}
			}
			else if (available == 0)
			{
				this_thread::sleep_for(50ms);
			}
			else if (available < 0)
			{
				this->logger->log("Error: Serial available < 0.");
			}
		#else
			this_thread::sleep_for(50ms);
		#endif
	}
	this->logger->log("Should-stop flag noticed.");
	this->stopped = true;
}

bool GPS::is_valid(string frame)
{
	regex frame_regex("\\$[A-Z][0-9A-Z\\.,-]*\\*[0-9A-F]{1,2}");
	if ( ! regex_match(frame, frame_regex)) return false;

	uint_fast8_t checksum = 0;
	for (char c : frame)
	{
		if (c == '$') continue;
		if (c == '*') break;

		checksum ^= c;
	}
	uint_fast8_t frame_cs = stoi(frame.substr(frame.rfind('*')+1, frame.length()-frame.rfind('*')-1), 0, 16);

	return checksum == frame_cs;
}

void GPS::parse(const string& frame)
{
	if (frame.length() > 1)
	{
		this->frame_logger->log(frame);
		string frame_type = frame.substr(1, frame.find_first_of(',')-1);

		if (frame_type == "GPGGA")
		{
			this->parse_GGA(frame);
		}
		else if (frame_type == "GPGSA")
		{
			this->parse_GSA(frame);
		}
		else if (frame_type == "GPRMC")
		{
			this->parse_RMC(frame);
		}
	}
}

void GPS::parse_GGA(const string& frame)
{
	stringstream ss(frame);
	string data;
	vector<string> s_data;

	// We put all fields in a vector
	while(getline(ss, data, ',')) s_data.push_back(data);

	// Is the data valid?
	this->active = s_data[6] > "0";

	if (this->active)
	{
		// Update time
		this->time.tm_hour = stoi(s_data[1].substr(0, 2));
		this->time.tm_min = stoi(s_data[1].substr(2, 2));
		this->time.tm_sec = stoi(s_data[1].substr(4, 2));

		// Update latitude
		this->latitude = stoi(s_data[2].substr(0, 2));
		this->latitude += stof(s_data[2].substr(2, s_data[2].length()-2))/60;
		if (s_data[3] == "S") this->latitude *= -1;

		// Update longitude
		this->longitude = stoi(s_data[4].substr(0, 3));
		this->longitude += stof(s_data[4].substr(3, s_data[4].length()-3))/60;
		if (s_data[5] == "W") this->longitude *= -1;

		// Update the rest of the GGA data
		this->satellites = stoi(s_data[7]);
		this->hdop = stof(s_data[8]);
		this->altitude = stod(s_data[9]);
	}
}

void GPS::parse_GSA(const string& frame)
{
	stringstream ss(frame);
	string data;
	vector<string> s_data;

	// We put all fields in a vector
	while(getline(ss, data, ',')) s_data.push_back(data);

	// Is the data valid?
	this->active = s_data[2] != "1";

	if (this->active)
	{
		// Update DOP
		this->pdop = stof(s_data[15]);
		this->hdop = stof(s_data[16]);
		this->vdop = stof(s_data[17].substr(0, s_data[17].find_first_of('*')));
	}
}

void GPS::parse_RMC(const string& frame)
{
	stringstream ss(frame);
	string data;
	vector<string> s_data;

	// We put all fields in a vector
	while(getline(ss, data, ',')) s_data.push_back(data);

	// Is the data valid?
	this->active = s_data[2] == "A";

	if (this->active)
	{
		// Update date and time
		this->time.tm_hour = stoi(s_data[1].substr(0, 2));
		this->time.tm_min = stoi(s_data[1].substr(2, 2));
		this->time.tm_sec = stoi(s_data[1].substr(4, 2));

		this->time.tm_mday = stoi(s_data[9].substr(0, 2));
		this->time.tm_mon = stoi(s_data[9].substr(2, 2))-1;
		this->time.tm_year = stoi(s_data[9].substr(4, 2))+100;

		// Update latitude
		this->latitude = stoi(s_data[3].substr(0, 2));
		this->latitude += stof(s_data[3].substr(2, s_data[3].length()-2))/60;
		if (s_data[4] == "S") this->latitude *= -1;

		// Update longitude
		this->longitude = stoi(s_data[5].substr(0, 3));
		this->longitude += stof(s_data[5].substr(3, s_data[5].length()-3))/60;
		if (s_data[6] == "W") this->longitude *= -1;

		// Update velocity
		this->velocity.speed = kt_to_mps(stof(s_data[7]));
		this->velocity.course = stof(s_data[8]);
	}
}
