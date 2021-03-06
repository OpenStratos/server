#include "gps/GPS.h"
#include "constants.h"

#include <functional>
#include <vector>
#include <sstream>
#include <string>
#include <regex>
#include <thread>

#include <sys/time.h>

#include <wiringPi.h>

#include "constants.h"
#include "serial/Serial.h"
#include "logger/Logger.h"


using namespace std;
using namespace os;

GPS& GPS::get_instance()
{
	static GPS instance;
	return instance;
}

GPS::~GPS()
{
	if ( ! this->stopped)
	{
		this->logger->log("Stopping GPS thread...");
		this->should_stop = true;
		while ( ! this->stopped)
		{
			this_thread::sleep_for(1ms);
		}
		this->logger->log("GPS thread stopped");
	}

	if (this->serial->is_open())
	{
		this->logger->log("Closing serial interface...");
		this->serial->close();
		this->logger->log("Serial interface closed.");
		this->logger->log("Deallocating serial...");
		delete this->serial;
		this->logger->log("Serial deallocated.");

		this->logger->log("Deallocating frame logger...");
		delete this->frame_logger;
		this->logger->log("Frame logger deallocated.");
	}
	if (this->is_on())
	{
		this->logger->log("Turning off GPS...");
		this->turn_off();
		this->logger->log("GPS off.");
	}
	delete this->logger;
}

bool GPS::initialize()
{
	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm * now = gmtime(&timer.tv_sec);

	this->logger = new Logger("data/logs/GPS/GPS."+ to_string(now->tm_year+1900) +"-"+ to_string(now->tm_mon+1) +"-"+
		to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+ to_string(now->tm_min) +"-"+
		to_string(now->tm_sec) +".log", "GPS");

	this->frame_logger = new Logger("data/logs/GPS/GPSFrames."+ to_string(now->tm_year+1900) +"-"+
		to_string(now->tm_mon+1) +"-"+ to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+
		to_string(now->tm_min) +"-"+ to_string(now->tm_sec) +".log", "GPSFrame");

	this->should_stop = false;
	this->stopped = true;

	this->time = timer;

	#ifndef OS_TESTING
		pinMode(GPS_ENABLE_GPIO, OUTPUT);

		if (this->is_on())
		{
			this->logger->log("GPS is on, turning off for 2 seconds for stability");
			this->turn_off();
			this_thread::sleep_for(2s);
		}

		this->logger->log("Turning GPS on...");
		this->turn_on();
		this->logger->log("GPS on.");
	#endif

	this->logger->log("Starting serial connection...");
	#ifdef DEBUG
		this->serial = new Serial(GPS_UART, GPS_BAUDRATE, "GPS");
	#else
		this->serial = new Serial(GPS_UART, GPS_BAUDRATE);
	#endif
	if ( ! this->serial->is_open())
	{
		this->logger->log("GPS serial error.");
		return false;
	}
	this->logger->log("Serial connection started.");

	#ifndef OS_TESTING
		this->logger->log("Sending configuration frames...");

		vector<uint8_t> messages[] =
		{
			// Set refresh:
			{ 0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0x64, 0x00, 0x01, 0x00, 0x01, 0x00, 0x7A, 0x12 },
			// Disable GSV:
			{ 0xB5, 0x62, 0x05, 0x01, 0x02, 0x00, 0x06, 0x01, 0x0F, 0x38 },
			// Disable VTG:
			{ 0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x01, 0x00, 0xFB, 0x11 },
			// Disable GLL:
			{ 0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x03, 0x00, 0xFD, 0x15 },
			// Disable ZDA:
			{ 0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x05, 0x00, 0xFF, 0x19 },
		};

		for (vector<uint8_t> mes: messages)
		{
			for(uint8_t tries = 0; tries<100; tries++)
			{
				this->serial->write_vec(mes);
				this_thread::sleep_for(10ms);
			}
		}
		this->logger->log("Configuration frames sent.");

		this->logger->log("Setting GPS to airborne (<1g) mode");
		if (this->enter_airborne_1g_mode())
		{
			this->logger->log("GPS entered airborne (<1g) mode successfully");
		}
		else
		{
			this->logger->log("GPS failed to enter airborne (<1g) mode");
		}
	#endif

	this->logger->log("Starting GPS frame thread...");
	thread t(&GPS::gps_thread, this);
	t.detach();
	this->logger->log("GPS frame thread running.");

	return true;
}

bool GPS::is_on() const
{
	return digitalRead(GPS_ENABLE_GPIO) == HIGH;
}

bool GPS::turn_on() const
{
	if ( ! this->is_on())
	{
		digitalWrite(GPS_ENABLE_GPIO, HIGH);
		return true;
	}
	else
	{
		this->logger->log("Warning: Turning on GPS but GPS already on.");
		return false;
	}
}

bool GPS::turn_off() const
{
	if (this->is_on())
	{
		digitalWrite(GPS_ENABLE_GPIO, LOW);
		return true;
	}
	else
	{
		this->logger->log("Warning: Turning off GPS but GPS already off.");
		return false;
	}
}

void GPS::gps_thread()
{
	this->stopped = false;
	string response;

	while( ! this->should_stop)
	{
		#ifndef OS_TESTING
			int available = this->serial->available();

			if (available > 0)
			{
				for (int i = 0; i < available; ++i)
				{
					char c = this->serial->read_char();
					response += c;
					if (response.length() > 2 &&
						response[response.length()-2] == '\r' &&
						c == '\n')
					{
						response = response.substr(0, response.length()-2);

						if (response.at(0) == '$')
						{
							this->parse(response);
						}
						response = "";
						this_thread::sleep_for(50ms);
					}
				}
			}
			else if (available == 0)
			{
				this_thread::sleep_for(50ms);
			}
			else if (available < 0)
			{
				this->logger->log("Error: Serial available < 0.");
			}
		#else
			this_thread::sleep_for(50ms);
		#endif
	}
	this->logger->log("Should-stop flag noticed.");
	this->stopped = true;
}

bool GPS::is_valid(string frame)
{
	regex frame_regex("\\$[A-Z][0-9A-Z\\.,-]*\\*[0-9A-F]{1,2}");
	if ( ! regex_match(frame, frame_regex))
	{
		return false;
	}

	uint_fast8_t checksum = 0;
	for (char c : frame)
	{
		if (c == '$')
		{
			continue;
		}
		if (c == '*')
		{
			break;
		}

		checksum ^= c;
	}
	uint_fast8_t frame_cs = stoi(frame.substr(frame.rfind('*')+1, frame.length()-frame.rfind('*')-1), 0, 16);

	return checksum == frame_cs;
}

void GPS::parse(const string& frame)
{
	if (frame.length() > 1 && this->is_valid(frame))
	{
		struct timeval os_clock;
		gettimeofday(&os_clock, NULL);
		double os_time = (double) os_clock.tv_sec + os_clock.tv_usec * 0.000001;

		this->frame_logger->log(frame);
		string frame_type = frame.substr(3, frame.find_first_of(',')-3);

		try
		{
			if (frame_type == "GGA")
			{
				this->parse_GGA(frame);
			}
			else if (frame_type == "GSA")
			{
				this->parse_GSA(frame);
			}
			else if (frame_type == "RMC")
			{
				this->parse_RMC(frame);
			}
		}
		catch (const invalid_argument& ia)
		{
			#ifdef DEBUG
				this->logger->log("Failed to parse frame: " + frame);
			#endif
		}

		double gps_time = (double) this->time.tv_sec + this->time.tv_usec * 0.000001;
		if (this->active && (gps_time > os_time + 5 || gps_time < os_time - 5))
		{
			#ifndef OS_TESTING
				settimeofday(&this->time, NULL);
			#endif

			this->logger->log("Date changed.");
		}
	}
}

void GPS::parse_GGA(const string& frame)
{
	stringstream ss(frame);
	string data;
	vector<string> s_data;

	string lat, lat_dec, lon, lon_dec;

	// We put all fields in a vector
	while(getline(ss, data, ','))
	{
		s_data.push_back(data);
	}

	if (s_data.size() != 15)
	{
		#ifdef DEBUG
			this->logger->log("Failed to parse frame: " + frame);
		#endif
		return;
	}

	// Is the data valid?
	bool active = s_data[6] == "1" || s_data[6] == "2";

	if (this->active && ! active)
	{
		this->logger->log("Fix lost.");
	}
	else if ( ! this->active && active)
	{
		this->logger->log("Fix acquired.");
	}
	this->active = active;

	double hdop;
	if (this->active && ! s_data[8].empty() && (hdop = stof(s_data[8])) < FAIR_DOP)
	{
		if (s_data[1].length() >= 6) // Update time
		{
			tm* time = gmtime(&this->time.tv_sec);
			time->tm_hour = stoi(s_data[1].substr(0, 2));
			time->tm_min = stoi(s_data[1].substr(2, 2));
			time->tm_sec = stoi(s_data[1].substr(4, 2));

			this->time.tv_sec = mktime(time);
			this->time.tv_usec = s_data[1].length() > 6 ? stoi(s_data[1].substr(7, 2))*10000 : 0;
		}

		if (s_data[2].length() > 2)
		{
			lat = s_data[2].substr(0, 2);
			lat_dec = s_data[2].substr(2, s_data[2].length()-2);

			this->latitude = stoi(lat);
			if( ! lat_dec.empty())
			{
				this->latitude += stof(lat_dec)/60;
			}
			if (s_data[3] == "S")
			{
				this->latitude *= -1;
			}
		}

		if (s_data[4].length() > 3)
		{
			lon = s_data[4].substr(0, 3);
			lon_dec = s_data[4].substr(3, s_data[4].length()-3);

			this->longitude = stoi(lon);
			if( ! lon_dec.empty())
			{
				this->longitude += stof(lon_dec)/60;
			}
			if (s_data[5] == "W")
			{
				this->longitude *= -1;
			}
		}

		// Validate and update the rest of the GGA data
		if( ! s_data[7].empty())
		{
			this->satellites = stoi(s_data[7]);
		}

		this->hdop = hdop;

		if( ! s_data[9].empty())
		{
			this->altitude = stod(s_data[9]);
		}
	}
}

void GPS::parse_GSA(const string& frame)
{
	stringstream ss(frame);
	string data;
	vector<string> s_data;

	// We put all fields in a vector
	while(getline(ss, data, ','))
	{
		s_data.push_back(data);
	}

	if (s_data.size() != 18)
	{
		#ifdef DEBUG
			this->logger->log("Failed to parse frame: " + frame);
		#endif
		return;
	}

	// Is the data valid?
	bool active = s_data[2] == "2" || s_data[2] == "3";

	if (this->active && ! active)
	{
		this->logger->log("Fix lost.");
		this->active = false;
	}
	else if ( ! this->active && active)
	{
		this->logger->log("Fix acquired.");
		this->active = true;
	}


	if (this->active && ! s_data[15].empty() && (pdop = stof(s_data[15])) < FAIR_DOP)
	{
		double pdop = FAIR_DOP+100, hdop = FAIR_DOP+100, vdop = FAIR_DOP+100;
		// Update DOP
		if( ! s_data[15].empty())
		{
			pdop = stof(s_data[15]);
			if (pdop > FAIR_DOP)
			{
				return;
			}
		}
		if( ! s_data[16].empty())
		{
			hdop = stof(s_data[16]);
			if (hdop > FAIR_DOP)
			{
				return;
			}
		}
		string vdop_str = s_data[17].substr(0, s_data[17].find_first_of('*'));
		if( ! vdop_str.empty())
		{
			vdop = stof(vdop_str);
			if (vdop > FAIR_DOP)
			{
				return;
			}
		}

		this->pdop = pdop;
		this->hdop = hdop;
		this->vdop = vdop;
	}
}

void GPS::parse_RMC(const string& frame)
{
	stringstream ss(frame);
	string data;
	vector<string> s_data;

	string lat, lat_dec, lon, lon_dec;

	// We put all fields in a vector
	while(getline(ss, data, ','))
	{
		s_data.push_back(data);
	}

	if (s_data.size() != 13)
	{
		#ifdef DEBUG
			this->logger->log("Failed to parse frame: " + frame);
		#endif
		return;
	}

	// Is the data valid?
	bool active = s_data[2] == "A";

	if (this->active && ! active)
	{
		this->logger->log("Fix lost.");
		this->active = false;
	}
	else if ( ! this->active && active)
	{
		this->logger->log("Fix acquired");
		this->active = true;
	}

	if (this->active)
	{
		if (s_data[1].length() >= 6) // Update time
		{
			tm* time = gmtime(&this->time.tv_sec);
			time->tm_hour = stoi(s_data[1].substr(0, 2));
			time->tm_min = stoi(s_data[1].substr(2, 2));
			time->tm_sec = stoi(s_data[1].substr(4, 2));

			if (s_data[9].length() == 6) // Update date
			{
				time->tm_mday = stoi(s_data[9].substr(0, 2));
				time->tm_mon = stoi(s_data[9].substr(2, 2))-1;
				time->tm_year = stoi(s_data[9].substr(4, 2))+100;
			}

			this->time.tv_sec = mktime(time);
			this->time.tv_usec = s_data[1].length() > 6 ? stoi(s_data[1].substr(7, 2))*10000 : 0;
		}
		else if (s_data[9].length() == 6) // Update date
		{
			tm* time = gmtime(&this->time.tv_sec);

			time->tm_mday = stoi(s_data[9].substr(0, 2));
			time->tm_mon = stoi(s_data[9].substr(2, 2))-1;
			time->tm_year = stoi(s_data[9].substr(4, 2))+100;

			this->time.tv_sec = mktime(time);
		}

		if(s_data[3].length() > 2)
		{
			lat = s_data[3].substr(0, 2);
			lat_dec = s_data[3].substr(2, s_data[3].length()-2);

			this->latitude = stoi(lat);
			if( ! lat_dec.empty())
			{
				this->latitude += stof(lat_dec)/60;
			}
			if (s_data[4] == "S")
			{
				this->latitude *= -1;
			}
		}

		if(s_data[5].length() > 3)
		{
			lon = s_data[5].substr(0, 3);
			lon_dec = s_data[5].substr(3, s_data[5].length()-3);

			this->longitude = stoi(lon);
			if( ! lon_dec.empty())
			{
				this->longitude += stof(lon_dec)/60;
			}
			if (s_data[6] == "W")
			{
				this->longitude *= -1;
			}
		}

		// Check if non-empty, and if so, update velocity
		if( ! s_data[7].empty())
		{
			this->velocity.speed = kt_to_mps(stof(s_data[7]));
		}
		if( ! s_data[8].empty())
		{
			this->velocity.course = stof(s_data[8]);
		}
	}
}

bool GPS::enter_airborne_1g_mode()
{
	bool gps_dynamic_model_set_success = false;
	struct timeval time_now, time_start;
	long ms_now, ms_start;

	vector<uint8_t> setdm6 =
	{
		// Header, class, ID, Length
		0xB5, 0x62, 0x06, 0x24, 0x24, 0x00,
		// Payload:
		// Mask, Dynmodel, FixType
		0xFF, 0xFF, 0x06, 0x03,
		0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64,
		0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00,
		// Checksum
		0x16, 0xDC
	};

	gettimeofday(&time_start, NULL);
	ms_start = (long)((time_start.tv_sec)*1000 + (time_start.tv_usec)/1000);
	ms_now = ms_start;

	// Prevent lock and timeout if not set after six seconds
	while( ! gps_dynamic_model_set_success && (ms_now - ms_start)<6000)
	{
		gettimeofday(&time_now, NULL);
		ms_now = (long)((time_now.tv_sec)*1000 + (time_now.tv_usec)/1000);

		this->send_ublox_packet(setdm6);
		gps_dynamic_model_set_success = this->receive_check_ublox_ack(setdm6);
	}

	return gps_dynamic_model_set_success;
}

void GPS::send_ublox_packet(const vector<uint8_t> &message)
{
	if (this->serial->is_open())
	{
		this->serial->flush();
		this->serial->write_byte(0xFF);
		this_thread::sleep_for(500ms);
		this->serial->write_vec(message);
	}
	else
	{
		this->logger->log("Tried to send a packet, but the serial is closed");
	}
}

bool GPS::receive_check_ublox_ack(const vector<uint8_t> &message)
{
	uint8_t ack_packet[10];
	unsigned int bytes_ordered;
	uint8_t byte;
	struct timeval time_now, time_start;
	long ms_now, ms_start;

	ack_packet[0] = 0xB5;
	ack_packet[1] = 0x62;
	ack_packet[2] = 0x05;
	ack_packet[3] = 0x01;
	ack_packet[4] = 0x02;
	ack_packet[5] = 0x00;
	ack_packet[6] = message[2];
	ack_packet[7] = message[3];
	ack_packet[8] = 0x00;
	ack_packet[9] = 0x00;

	for (int i = 2; i<8; ++i)
	{
		ack_packet[8]+= ack_packet[i];
		ack_packet[9]+= ack_packet[8];
	}
	bytes_ordered = 0;

	gettimeofday(&time_start, NULL);
	ms_start = (long)((time_start.tv_sec)*1000 + (time_start.tv_usec)/1000);
	ms_now = ms_start;

	// Prevent lock and timeout if not set after three seconds
	while (ms_now - ms_start <= 3000)
	{
		if (bytes_ordered > 9)
		{
			return true;
		}

		gettimeofday(&time_now, NULL);
		ms_now = (long)((time_now.tv_sec)*1000 + (time_now.tv_usec)/1000);

		if (this->serial->available())
		{
			byte = this->serial->read_byte();
			if (byte == ack_packet[bytes_ordered])
			{
				bytes_ordered++;
			}
			else
			{
				bytes_ordered = 0;
			}
		}
	}
	return false;
}
