#include "gps/GPS.h"

#include <functional>
#include <vector>
#include <sstream>
#include <string>
#include <regex>

#include <sys/time.h>

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
	if (this->serial.is_open())
	{
		this->logger->log("Closing serial interface...");
		this->serial.close();
		this->logger->log("Serial interface closed.");

		this->logger->log("Deallocating frame logger...");
		delete this->frame_logger;
	}
	this->logger->log("Shutting down...");
	// TODO shut down GPS
	this->logger->log("Shut down finished.");
	delete this->logger;
}

bool GPS::initialize(const string& serial_URL)
{
	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm * now = gmtime(&timer.tv_sec);

	this->logger = new Logger("data/logs/GPS/GPS."+ to_string(now->tm_year+1900) +"-"+ to_string(now->tm_mon) +"-"+
		to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+ to_string(now->tm_min) +"-"+
		to_string(now->tm_sec) +".log", "GPS");

	this->logger->log("Starting serial connection...");
	if ( ! this->serial.initialize(serial_URL, 9600, "\r\n", bind(&GPS::parse, this, placeholders::_1))) {
		this->logger->log("GPS serial error.");
		return false;
	} else {
		this->logger->log("Serial connection started.");
		this->frame_logger = new Logger("data/logs/GPS/GPSFrames."+ to_string(now->tm_year+1900) +"-"+
			to_string(now->tm_mon) +"-"+ to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+
			to_string(now->tm_min) +"-"+ to_string(now->tm_sec) +".log", "GPS Frame");
		return true;
	}

	#ifndef OS_TESTING
		this->serial.send("$PMTK220,100*2F");
		this->serial.send("$PMTK314,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29");
	#endif
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

uint_fast8_t GPS::parse(const string& frame)
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

	return ERR_OK;
}

void GPS::parse_GGA(const string& frame)
{
	stringstream ss(frame);
	string data;
	vector<string> s_data;

	// We put all fields in a vector
	while(getline(ss, data, ','))
			s_data.push_back(data);

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
		this->time.tm_mon = stoi(s_data[9].substr(2, 2));
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
