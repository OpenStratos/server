#include "serial/Serial.h"

#include <cstdint>

#include <thread>
#include <regex>
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

Serial::~Serial()
{
	this->close();
}

void Serial::serial_thread()
{
	string frame;
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
					frame += c;

					if (c == this->endl[endl_pos]) ++endl_pos;
					if (endl_pos == this->endl.length())
					{
						frame = frame.substr(0, frame.length()-endl.length());

						if (this->is_valid(frame)) this->listener(frame);
						// TODO decide what to do in case of non valid frame

						frame = "";
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

uint_fast8_t Serial::send_frame(string frame)
{
	if (this->is_valid(frame))
	{
		frame += endl;
		serialPuts(this->fd, frame.c_str());
	}
	else
	{
		// TODO log error
	}
}

void Serial::close()
{
	this->open = false;
	while( ! this->stopped);

	#ifndef OS_TESTING
		serialClose(this->fd);
	#endif
}

bool Serial::is_valid(string frame)
{
	regex frame_regex("\\$[A-Z][0-9A-Z\\.,-]*\\*[0-9A-F]{1,2}");
	if ( ! regex_match(frame, frame_regex)) return false;

	uint_fast8_t checksum = 0;
	for (char c : frame)
	{
		if (c == '$') continue;
		if (c == '*') break;

		checksum ^= c;
	}
	uint_fast8_t frame_cs = stoi(frame.substr(frame.rfind('*')+1, frame.length()-frame.rfind('*')-1), 0, 16);

	return checksum == frame_cs;
}
