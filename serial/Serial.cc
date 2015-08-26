#include "serial/Serial.h"

#include <cstdint>

#include <thread>
#include <functional>
#include <string>

#include <sys/time.h>

#include <wiringSerial.h>

#include "constants.h"
#include "gps/GPS.h"
#if DEBUG
	#include "logger/Logger.h"
#endif

using namespace std;
using namespace os;

Serial::Serial(const string& url, int baud_rate, const string& log_path)
{
	this->open = false;

	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm * now = gmtime(&timer.tv_sec);

	#if DEBUG
		this->logger = new Logger("data/logs/"+log_path+"/Serial."+ to_string(now->tm_year+1900) +"-"+
			to_string(now->tm_mon) +"-"+ to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+
			to_string(now->tm_min) +"-"+ to_string(now->tm_sec) +".log", "Serial");
	#endif

	#ifndef OS_TESTING
		this->fd = serialOpen(url.c_str(), baud_rate);

		#if DEBUG
			if (this->fd == -1) this->logger->log("Error: connection fd is -1.");
			else this->open = true;
		#else
			if (this->fd != -1) this->open = true;
		#endif
	#endif
}

Serial::~Serial()
{
	if (this->open)
		this->close();

	#if DEBUG
		delete this->logger;
	#endif
}

void Serial::println(const string& str) const
{
	serialPuts(this->fd, (str+"\r\n").c_str());

	#if DEBUG
		this->logger->log("Sent: '"+str+"\\r\\n'");
	#endif
}

void Serial::println() const
{
	serialPuts(this->fd, "\r\n");

	#if DEBUG
		this->logger->log("Sent: '\\r\\n'");
	#endif
}

void Serial::write(unsigned char c) const
{
	serialPutchar(this->fd, c);

	#if DEBUG
		this->logger->log("Sent char: '"+string(1, c)+"'");
	#endif
}

void Serial::close()
{
	#ifndef OS_TESTING
		if (this->open) {
			serialClose(this->fd);
			this->open = false;
		}
	#endif
}

bool Serial::is_open() const
{
	return this->open;
}

int Serial::available() const
{
	return serialDataAvail(this->fd);
}

char Serial::read_char() const
{
	return serialGetchar(this->fd);
}

const string Serial::read_line() const
{
	return this->read_line(1);
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
				#if DEBUG
					this->logger->log("Error: Serial timeout. ("+to_string(timeout)+" s)");
				#endif

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
				#if DEBUG
					this->logger->log("Error: Serial available < 0.");
				#endif

				break;
			}
			this_thread::sleep_for(1ms);
		}
	#endif

	#if DEBUG
		this->logger->log("Received: '"+logstr+"'");
	#endif

	return response;
}

void Serial::flush() const
{
	serialFlush(this->fd);
}
