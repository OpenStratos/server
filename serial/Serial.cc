#include "serial/Serial.h"

#include <cstdint>

#include <thread>
#include <functional>
#include <string>

#include <sys/time.h>

#include <wiringSerial.h>

#include "constants.h"
#include "gps/GPS.h"

using namespace std;
using namespace os;

bool Serial::initialize_GPS()
{
	#ifndef OS_TESTING
		this->fd = serialOpen(GPS_UART, GPS_BAUDRATE);

		if (this->fd == -1) {
			this->open = false;
			this->stopped = true;
			return false;
		}

		this->open = true;
		this->stopped = false;
		thread t(&Serial::gps_thread, this);
		t.detach();
	#else
		this->open = false;
		this->stopped = true;
	#endif

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
	this->stopped = true;

	return true;
}

Serial::~Serial()
{
	this->close();
}

void Serial::gps_thread()
{
	string response;

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
					if (response[response.length()-1] == '\r' && c == '\n')
					{
						response = response.substr(0, response.length()-2);

						if (response.at(0) == '$')
						{
							GPS::get_instance().parse(response);
						}
						response = "";
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
	serialPuts(this->fd, (str+"\r\n").c_str());
}

void Serial::close()
{
	if (this->open) {
		this->open = false;
		while( ! this->stopped)
			this_thread::sleep_for(1ms);

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
	string response;

	#ifndef OS_TESTING
		int available = serialDataAvail(this->fd);
		struct timeval t1, t2;
		double elapsed_time = 0;

		while (true)
		{
			gettimeofday(&t1, NULL);
			while (available == 0)
			{
				this_thread::sleep_for(25ms);
				gettimeofday(&t2, NULL);
				elapsed_time = (t2.tv_sec - t1.tv_sec);
				elapsed_time += (t2.tv_usec - t1.tv_usec) / 1000000.0;

				if (elapsed_time > 5) return "";
				available = serialDataAvail(this->fd);
			}

			if (available < 0)
			{
				// TODO log error
			}

			for (int i = 0; i < available; i++)
			{
				char c = serialGetchar(this->fd);

				response += c;
				if (response[response.length()-1] == '\r' && c == '\n')
				{
					return response.substr(0, response.length()-2);
				}
			}
		}
	#endif
	return response;
}

bool Serial::read_only(const string& only) const
{
	int available = serialDataAvail(this->fd);
	struct timeval t1, t2;
	double elapsed_time = 0;

	while (true)
	{
		gettimeofday(&t1, NULL);
		while (available == 0)
		{
			this_thread::sleep_for(25ms);
			gettimeofday(&t2, NULL);
			elapsed_time = (t2.tv_sec - t1.tv_sec);
			elapsed_time += (t2.tv_usec - t1.tv_usec) / 1000000.0;

			if (elapsed_time > 5) return false;
			available = serialDataAvail(this->fd);
		}

		if (available >= only.length())
		{
			for (int i = 0; i < only.length(); i++)
			{
				if (serialGetchar(this->fd) != only[i]) return false;
			}
			return true;
		}
	}
}

void Serial::flush() const
{
	serialFlush(this->fd);
}
