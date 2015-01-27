#include "Serial.hpp"
#include <wiringSerial.h>
#include <thread>
#include <regex>

using namespace os;

Serial::Serial(const string& url, int baud, const string endl, function<uint_fast8_t(const string&)>)
{
	this->listener = listener;
	this->endl = endl;
	#ifndef OS_TESTING
		this->fd = serialOpen(url.c_str(), baud);
		this->open = true;

		thread t(&Serial::serial_thread, this);
		t.detach();
	#endif
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
		this_thread::sleep_for(chrono::milliseconds(1));
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
					this_thread::sleep_for(chrono::milliseconds(50));
				}
			}
		}
		else if (available < 0)
		{
			// TODO check error and throw exception
		}
	}
}

uint_fast8_t Serial::send_frame(string frame)
{
	if (this->is_valid(frame))
	{
		frame += endl;
		serialPuts(this->fd, frame.c_str());
	}
	// TODO what if it's not valid? exception?
}

void Serial::close()
{
	this->open = false;
	serialClose(this->fd);
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
