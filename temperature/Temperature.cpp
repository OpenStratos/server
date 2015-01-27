#include "Temperature.hpp"
#include "../constants.hpp"

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

		// 32768 = 2^15
		int voltage = value * 5 / 32768;
		
		int temperature = TEMP_R * (TEMP_VIN / voltage - 1);

		// TODO Refine formula
		int temp = temperature - 1000 / 3.91;

		this->lasTemp = temp;
		this_thread::sleep_for(chrono::milliseconds(50));
	}
}
