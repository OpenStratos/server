#include "serial/Serial.h"

#include <cstdint>

#include <thread>
#include <functional>
#include <string>

#include <wiringSerial.h>

#include "constants.h"
#include "gps/GPS.h"

using namespace std;
using namespace os;

bool Serial::initialize_GPS()
{
	this->endl = GPS_ENDL;
	#ifndef OS_TESTING
		this->fd = serialOpen(GPS_UART, GPS_BAUDRATE);

		if (this->fd == -1) {
			this->open = false;
			this->stopped = true;
			return false;
		}
	#endif

	this->open = true;
	this->stopped = false;
	thread t(&Serial::gps_thread, this);
	t.detach();

	return true;
}

bool Serial::initialize(const string& url, int baud)
{
	#ifndef OS_TESTING
		this->fd = serialOpen(url.c_str(), baud);
	#endif

	if (this->fd == -1)
	{
		return false;
	}

	this->open = true;
	this->stopped = false;

	return true;
}

Serial::~Serial()
{
	this->close();
}

void Serial::gps_thread()
{
	string response;
	int endl_pos = 0;

	while(this->open)
	{
		#ifndef OS_TESTING
			int available = serialDataAvail(this->fd);

			if (available > 0)
			{
				for (int i = 0; i < available; i++)
				{
					char c = serialGetchar(this->fd);
					response += c;
					if (c == this->endl[endl_pos]) ++endl_pos;
					if (endl_pos == this->endl.length())
					{
						response = response.substr(0, response.length()-endl.length());

						if (response.at(0) == '$')
						{
							GPS::get_instance().parse(response);
						}
						response = "";
						endl_pos = 0;
						this_thread::sleep_for(50ms);
					}
				}
			}
			else if (available == 0)
			{
				this_thread::sleep_for(25ms);
			}
			else if (available < 0)
			{
				// TODO log error
			}
		#endif
	}
	this->stopped = true;
}

void Serial::send(const string& str) const
{
	serialPuts(this->fd, (str+this->endl).c_str());
}

void Serial::close()
{
	if (this->open) {
		this->open = false;
		while( ! this->stopped);

		#ifndef OS_TESTING
			serialClose(this->fd);
		#endif
	}
}

bool Serial::is_open()
{
	return this->open;
}

const string Serial::read_line() const
{
	// TODO
	string response;
	return response;
}

void Serial::flush() const
{
	serialFlush(this->fd);
}
