#include "temperature/Temperature.h"

#include <string>
#include <thread>

#include <wiringPiI2C.h>

#include "constants.h"

using namespace std;
using namespace os;

Temperature::~Temperature()
{
	if (this->reading)
		this->stop_reading();
}

Temperature::Temperature(const int address)
{
	this->reading = false;
	this->stopped = true;
	#ifndef OS_TESTING
		int fh = wiringPiI2CSetup(address);
		if (fh != -1)
		{
			this->address = address;
			this->filehandle = fh;
		}
		else
		{
			// TODO Log error
			//printf("An error ocurred initializing I2C Temperature module\n");
		}
	#endif
}

void Temperature::start_reading()
{
	if ( ! this->reading)
	{
		this->reading = true;
		this->stopped = false;
		thread t(&Temperature::read_temperature, this);
		t.detach();
	}
}

void Temperature::stop_reading()
{
	this->reading = false;
	while( ! this->stopped);
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

		float voltage = value * 5 / 32768; // 2^15
		this->temperature = r_to_c(TEMP_R * (TEMP_VIN / voltage - 1));

		this_thread::sleep_for(50ms);
	}
	this->stopped = true;
}
