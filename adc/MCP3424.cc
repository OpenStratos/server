#include "adc/MCP3424.h"
#include <wiringPiI2C.h>

using namespace os;

MCP3424::MCP3424(int i2c_address, int gain, int res)
{
	this->gain = gain;
	this->res = res;
	this->fd = wiringPiI2CSetup(i2c_address);
}
