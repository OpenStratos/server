#include "GPS.hpp"
#include <wiringSerial.h>

using namespace os;

GPS::~GPS()
{
	// TODO Stop thread, close serial etc.
}

void GPS::initialize(Serial serial)
{
	this->serial = serial;
	thread t(&GPS::serialPoll, this);
	// TODO configure GPS
}

void GPS::serialPoll()
{
	// TODO
}

void GPS::parse(string frame)
{
	// TODO
}
