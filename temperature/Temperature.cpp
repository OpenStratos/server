#include "Temperature.hpp"
#include "../constants.hpp"
#include <wiringPiI2C.h>
#include <string>
#include <thread>
#include <chrono>

using namespace os;

Temperature::~Temperature()
{
	if (this->reading)
		this->stop_reading();
}

Temperature::Temperature(const int devId)
{
	this->reading = false;
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

void Temperature::start_reading()
{
	if (!this->reading)
	{
		thread t(&Temperature::read_temperature, this);
		this->reading = true;
		t.detach();
	}
}

void Temperature::stop_reading()
{
	this->reading = false;
}

void Temperature::read_temperature()
{
	while (this->reading)
	{
		int value = wiringPiI2CRead(this->filehandle);

		// 32768 = 2^15
		float voltage = value * 5 / 32768;
		
		float temperature = TEMP_R * (TEMP_VIN / voltage - 1);


		float temp = r_to_c(temperature);

		this->lastTemp = temp;
		this_thread::sleep_for(chrono::milliseconds(50));
	}
}
