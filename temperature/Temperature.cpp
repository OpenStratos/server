#include "Temperature.hpp"

using namespace os;

Temperature::~Temperature()
{
	if (this->reading)
		this->stopReading();
}

void Temperature::initialize(const int devId)
{
	int fh = wiringPiI2CSetup(devId);
	if (fh != -1)
	{
		this->devId = devId;
		this->filehandle = fh;
	}
	else
	{	
		printf("An error ocurred initializing I2C Temperature module\n");
	}
}

void Temperature::startReading()
{
	this->readThread = thread t(&Temperature::readTemperature, this);
	this->reading = true;
}

void Temperature::stopReading()
{
	this->reading = false;
	this->readThread.detach();
}

void Temperature::readTemperature()
{
	while (this->reading) {
		int value = wiringPiI2CRead(this->filehandle);
		//TODO Convert value to temperature
		this->lasTemp = value;
		this_thread::sleep_for(chrono::milliseconds(50));
	}
}
