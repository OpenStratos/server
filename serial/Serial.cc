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
	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm * now = gmtime(&timer.tv_sec);

	this->logger = new Logger("data/logs/GPS/Serial."+ to_string(now->tm_year+1900) +"-"+ to_string(now->tm_mon) +"-"+
		to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+ to_string(now->tm_min) +"-"+
		to_string(now->tm_sec) +".log", "GPSSerial");

	#ifndef OS_TESTING
		this->fd = serialOpen(GPS_UART, GPS_BAUDRATE);

		if (this->fd == -1) {
			this->open = false;
			this->stopped = true;

			return false;
		}

		this->open = true;
		this->stopped = false;
		this->flush();
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
	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm * now = gmtime(&timer.tv_sec);

	this->logger = new Logger("data/logs/GSM/Serial."+ to_string(now->tm_year+1900) +"-"+ to_string(now->tm_mon) +"-"+
		to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+ to_string(now->tm_min) +"-"+
		to_string(now->tm_sec) +".log", "GSMSerial");

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
	delete this->logger;
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
						this->logger->log("Received: '"+response+"\\r\\n'");

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
				this->logger->log("Error: Serial available < 0.");
			}
		#endif
	}
	this->stopped = true;
}

void Serial::println(const string& str) const
{
	this->logger->log("Sent: '"+str+"\\r\\n'");
	serialPuts(this->fd, (str+"\r\n").c_str());
}

void Serial::println() const
{
	this->logger->log("Sent: '\\r\\n'");
	serialPuts(this->fd, "\r\n");
}

void Serial::write(const string& str) const
{
	this->logger->log("Sent: '"+str+"'");
	serialPuts(this->fd, str.c_str());
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
	return this->read_line(0.5);
}

const string Serial::read_line(double timeout) const
{
	string response = "";
	string logstr = "";
	bool rfound = false, endl_found = false;
	int available = 0;

	#ifndef OS_TESTING
		struct timeval t1, t2;
		double elapsed_time = 0;
		gettimeofday(&t1, NULL);

		while ( ! endl_found)
		{
			gettimeofday(&t2, NULL);
			elapsed_time = (t2.tv_sec - t1.tv_sec);
			elapsed_time += (t2.tv_usec - t1.tv_usec) / 1000000.0;

			if (elapsed_time > timeout)
			{
				this->logger->log("Error: Serial timeout. ("+to_string(timeout)+" s)");
				break;
			}

			while (available = serialDataAvail(this->fd) > 0)
			{
				char c = serialGetchar(this->fd);

				if (c == '\r') logstr += "\\r";
				else if (c == '\n') logstr += "\\n";
				else logstr += c;

				if (c == '\r')
				{
					rfound = true;
					continue;
				}
				else if (c == '\n')
				{
					if (rfound)
					{
						endl_found = true;
						break;
					}
				}

				rfound = false;
				response += c;
			}

			if (available < 0)
			{
				this->logger->log("Error: Serial available < 0.");
				break;
			}
		}
	#endif
	this->logger->log("Received: '"+logstr+"'");
	return response;
}

void Serial::flush() const
{
	serialFlush(this->fd);
}
