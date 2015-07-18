#include "serial/Serial.h"

#include <cstdint>

#include <thread>
#include <functional>
#include <string>
#include <iostream>

#include <wiringSerial.h>

using namespace std;
using namespace os;

void Serial::initialize(const string& url, int baud, const string endl, function<uint_fast8_t(const string&)>)
{
	this->listener = listener;
	this->endl = endl;
	#ifndef OS_TESTING
		this->fd = serialOpen(url.c_str(), baud);
	#endif

	this->open = true;
	this->stopped = false;
	thread t(&Serial::serial_thread, this);
	t.detach();
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

void Serial::serial_thread()
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

						this->listener(response);
						response = "";
						endl_pos = 0;
						this_thread::sleep_for(chrono::milliseconds(50));
					}
				}
			}
			else if (available == 0)
			{
				this_thread::sleep_for(chrono::milliseconds(25));
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
	this->open = false;
	while( ! this->stopped);

	#ifndef OS_TESTING
		serialClose(this->fd);
	#endif
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
