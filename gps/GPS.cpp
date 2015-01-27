#include "../constants.hpp"
#include "GPS.hpp"
#include "../serial/Serial.hpp"
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

	return ERR_OK;
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
	this->longitude += stof(s_data[4].substr(2, s_data[4].length()-1));
	if (s_data[5] == "W") this->latitude *= -1;

	// Update the rest of the GGA data
	this->active = stoi(s_data[6]) > 0;
	this->satellites = stoi(s_data[7]);
	this->hdop = stof(s_data[8]);
	this->altitude = stof(s_data[9]);
}

void GPS::parse_GSA(const string& frame)
{
	stringstream ss(frame);
	string data;
	vector<string> s_data(18); // GSA has a total of 18 data fields

	// We put all fields in a vector
	while (getline(ss, data, ',')) s_data.push_back(data);

	// Update fix status
	if (s_data[2] == "1") this->active = false;

	// Update DOP
	this->hdop = stof(s_data[16]);
	this->vdop = stof(s_data[17].substr(0, s_data[17].find_first_of('*')));
}

void GPS::parse_RMC(const string& frame)
{
	stringstream ss(frame);
	string data;
	vector<string> s_data(12); // RMC has a total of 12 data fields

	// We put all fields in a vector
	while (getline(ss, data, ',')) s_data.push_back(data);

	// Update date and time
	tm* current_t = gmtime(&this->time);
	current_t->tm_hour = stoi(s_data[1].substr(0, 1));
	current_t->tm_min = stoi(s_data[1].substr(2, 3));
	current_t->tm_sec = stoi(s_data[1].substr(4, 5));

	current_t->tm_mday = stoi(s_data[9].substr(0, 1));
	current_t->tm_mon = stoi(s_data[9].substr(2, 3));
	current_t->tm_year = stoi(s_data[9].substr(4, 5))+100;
	this->time = mktime(current_t);

	// Update fix status
	this->active = s_data[2] == "A";

	// Update latitude
	this->latitude = stoi(s_data[3].substr(0, 1));
	this->latitude += stof(s_data[3].substr(2, s_data[3].length()-1));
	if (s_data[4] == "S") this->latitude *= -1;

	// Update longitude
	this->longitude = stoi(s_data[5].substr(0, 1));
	this->longitude += stof(s_data[5].substr(2, s_data[5].length()-1));
	if (s_data[6] == "W") this->latitude *= -1;

	// Update velocity
	this->velocity = knots_to_mps(stof(s_data[7]));
}
