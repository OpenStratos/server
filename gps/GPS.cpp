#include "GPS.hpp"

#include <wiringSerial.h>

using namespace os;

GPS::GPS(string serialURL)
{
	this->fd = serialOpen(serialURL.c_str(), 9600);
	// TODO create thread etc.
}

GPS::~GPS()
{
	serialClose(this->fd);
	// TODO Stop thread etc.
}

void GPS::serialPoll()
{
	// TODO
}

void GPS::parse(string frame)
{
	// TODO
}

void GPS::setActive(bool active)
{
	this->active = active;
}

void GPS::setCoordinate(coordinate c)
{
	this->coord = c;
}

void GPS::setVelocity(float v)
{
	this->velocity = v;
}

void GPS::setAngle(float a)
{
	this->angle = a;
}

void GPS::setAltitude(float a)
{
	this->altitude = a;
}

bool GPS::isActive() const
{
	return this->active;
}

coordinate GPS::getCoordinate() const
{
	return this->coord;
}

float GPS::getVelocity() const
{
	return this->velocity;
}

float GPS::getAngle() const
{
	return this->angle;
}

float GPS::getAltitude() const
{
	return this->altitude;
}
