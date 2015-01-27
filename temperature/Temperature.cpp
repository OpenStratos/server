#include "Temperature.hpp"
#include "../constants.hpp"

using namespace os;

Temperature& Temperature::get_instance()
{
	static Temperature instance;
	return instance;
}

Temperature::~Temperature()
{
	if (this->reading)
		this->stop_reading();
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

		// TODO Refine formula
		float temp = temperature - 1000 / 3.91;

		this->lastTemp = temp;
		this_thread::sleep_for(chrono::milliseconds(50));
	}
}
