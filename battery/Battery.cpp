#include "Battery.hpp"
#include "../constants.hpp"
#include <wiringPiI2C.h>
#include <string>
#include <thread>
#include <chrono>
#include "../constants.hpp"

using namespace std;
using namespace os;

Battery::~Battery()
{
	if (this->reading)
		this->stop_reading();
}

Battery::Battery(const int address)
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
			//printf("An error ocurred initializing I2C Battery module\n");
		}
	#endif
}

void Battery::start_reading()
{
	if ( ! this->reading)
	{
		this->reading = true;
		this->stopped = false;
		thread t(&Battery::read_battery, this);
		t.detach();
	}
}

void Battery::stop_reading()
{
	this->reading = false;
	while( ! this->stopped);
}

void Battery::read_battery()
{
	while (this->reading)
	{
		#ifndef OS_TESTING
			int value = wiringPiI2CRead(this->filehandle);
		#else
			int value = 16000;
		#endif

		float voltage5 = value * 5 / 32768; // 2^15

		this->battery = volt_to_percent(voltage5*(BAT_R1+BAT_R2)/BAT_R2);

		this_thread::sleep_for(chrono::milliseconds(50));
	}
	this->stopped = true;
}
