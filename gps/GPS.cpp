#include "GPS.hpp"
#include <wiringSerial.h>
#include <functional>

using namespace os;

GPS::~GPS()
{
	this->serial.close();
}

void GPS::initialize(const string& serial_URL)
{
	Serial serial(serial_URL, 9600, "\r\n", bind(&GPS::parse, this, placeholders::_1));
	this->serial = serial;
}

uint_fast8_t GPS::parse(const string& frame)
{
	// TODO parse frame and update GPS
}
