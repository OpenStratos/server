#include "GPS.hpp"
#include <wiringSerial.h>
#include <functional>
#include <vector>
#include <sstream>

using namespace os;

GPS::~GPS()
{
	this->serial.close();
}

void GPS::initialize(const string& serial_URL)
{
	Serial serial(serial_URL, 9600, "\r\n", bind(&GPS::parse, this, placeholders::_1));
	this->serial = serial;
	// TODO setup GPS with commands
}

uint_fast8_t GPS::parse(const string& frame)
{
	string frame_type = frame.substr(1, frame.find_first_of(','));

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

void GPS::parse_GGA(const string& frame)
{
	stringstream ss(frame);
	string data;
	vector<string> s_data(16); // GGA has a total of 16 data fields

	// We put all fields in a vector
	while (getline(ss, data, ',')) s_data.push_back(data);

	// Update time
	tm* current_t = gmtime(&this->time);
	current_t->tm_hour = stoi(s_data[1].substr(0, 1));
	current_t->tm_min = stoi(s_data[1].substr(2, 3));
	current_t->tm_sec = stoi(s_data[1].substr(4, 5));
	this->time = mktime(current_t);

	// Update latitude
	this->latitude = stoi(s_data[2].substr(0, 1));
	this->latitude += stof(s_data[2].substr(2, s_data[2].length()-1));
	if (s_data[3] == "S") this->latitude *= -1;

	// Update longitude
	this->longitude = stoi(s_data[4].substr(0, 1));
	this->longitude += stof(s_data[4].substr(2, s_data[2].length()-1));
	if (s_data[5] == "W") this->latitude *= -1;

	// Update the rest of the GGA data
	this->active = stoi(s_data[6]) > 0;
	this->satellites = stoi(s_data[7]);
	this->hdop = stof(s_data[8]);
	this->altitude = stof(s_data[9]);
}

void GPS::parse_GSA(const string& frame)
{
	// TODO
}

void GPS::parse_RMC(const string& frame)
{
	// TODO
}
