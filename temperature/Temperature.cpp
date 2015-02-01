#include "Temperature.hpp"
#include "../constants.hpp"
#include <wiringPiI2C.h>
#include <string>
#include <thread>
#include <chrono>
#include <iostream>

using namespace std;
using namespace os;

Temperature::~Temperature()
{
	if (this->reading)
		this->stop_reading();
}

Temperature::Temperature(const int devId)
{
	this->reading = false;
	#ifndef OS_TESTING
		int fh = wiringPiI2CSetup(devId);
		if (fh != -1)
		{
			this->devId = devId;
			this->filehandle = fh;
		}
		else
		{
			//TODO Log error
			//printf("An error ocurred initializing I2C Temperature module\n");
		}
	#endif
}

void Temperature::start_reading()
{
	if ( ! this->reading)
	{
		this->reading = true;
		thread t(&Temperature::read_temperature, this);
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
		#ifndef OS_TESTING
			int value = wiringPiI2CRead(this->filehandle);
		#else
			int value = 16000;
		#endif

		// 32768 = 2^15
		float voltage = value * 5 / 32768;

		float temperature = TEMP_R * (TEMP_VIN / voltage - 1);


		float temp = r_to_c(temperature);

		this->lastTemp = temp;
		this_thread::sleep_for(chrono::milliseconds(50));
	}
}
