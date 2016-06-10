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
	this->logger->log("Turning off GPS...");
	this->turn_off();
	this->logger->log("GPS off.");
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

	this->logger->log("Setting GPS to pedestrian mode");
	this->enter_pedestrian_mode();

	this->logger->log("Starting GPS frame thread...");
	thread t(&GPS::gps_thread, this);
	t.detach();
	this->logger->log("GPS frame thread running.");

	#ifndef OS_TESTING
		this->logger->log("Sending configuration frames...");

		char set_refresh[] =
			"\xB5\x62\x06\x08\x06\x00\x64\x00\x01\x00\x01\x00\x7A\x12";
		uint8_t sz_set_refresh = 14;

		char disable_gsv[] = "\xB5\x62\x05\x01\x02\x00\x06\x01\x0F\x38";
		uint8_t sz_disable_gsv = 10;

		char disable_vtg[] = "\xB5\x62\x06\x01\x03\x00\xF0\x01\x00\xFB\x11";
		uint8_t sz_disable_vtg = 11;

		char disable_gll[] = "\xB5\x62\x06\x01\x03\x00\xF0\x03\x00\xFD\x15";
		uint8_t sz_disable_gll = 11;

		char disable_zda[] = "\xB5\x62\x06\x01\x03\x00\xF0\x05\x00\xFF\x19";
		uint8_t sz_disable_zda = 11;

		char* messages[] =
		{
			set_refresh,disable_gsv,
			disable_vtg,disable_gll,
			disable_zda
		};
		uint8_t sizes[] =
		{
			sz_set_refresh,
			sz_disable_gsv,
			sz_disable_vtg,
			sz_disable_gll,
			sz_disable_zda
		};

		for(uint8_t message = 0; message<5; message++)
		{
			for(uint8_t tries = 0; tries<100; tries++)
			{
				for(uint8_t i = 0; i<sizes[message]; i++)
				{
					this->serial->write(messages[message][i]);
				}
			}
		}
		this->logger->log("Configuration frames sent.");
	#endif

	return true;
}

bool GPS::turn_on() const
{
	if (digitalRead(GPS_ENABLE_GPIO) == LOW)
	{
		digitalWrite(GPS_ENABLE_GPIO, HIGH);
		return true;
	}
	else
	{
		this->logger->log("Error: Turning on GPS but GPS already on.");
		return false;
	}
}

bool GPS::turn_off() const
{
	if (digitalRead(GPS_ENABLE_GPIO) == HIGH)
	{
		digitalWrite(GPS_ENABLE_GPIO, LOW);
		return true;
	}
	else
	{
		this->logger->log("Error: Turning off GPS but GPS already off.");
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
				for (int i = 0; i < available; i++)
				{
					char c = this->serial->read_char();
					response += c;
					if (response[response.length()-2] == '\r' && c == '\n')
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
	if (frame.length() > 1 && is_valid(frame))
	{
		struct timeval os_clock;
		gettimeofday(&os_clock, NULL);
		double os_time = (double) os_clock.tv_sec + os_clock.tv_usec * 0.000001;

		this->frame_logger->log(frame);
		string frame_type = frame.substr(3, frame.find_first_of(',')-3);

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

		double gps_time = (double) this->time.tv_sec + this->time.tv_usec * 0.000001;
		if (gps_time > os_time + 1 || gps_time < os_time - 1)
		{
			this->update_date();
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

	// Is the data valid?
	bool active = s_data[6] > "0";
	if (this->active && ! active)
	{
		this->logger->log("Fix lost.");
	}
	else if ( ! this->active && active)
	{
		this->logger->log("Fix acquired.");
	}
	this->active = active;

	if (s_data[1].length() >= 6) // Update time
	{
		tm* time = gmtime(&this->time.tv_sec);
		time->tm_hour = stoi(s_data[1].substr(0, 2));
		time->tm_min = stoi(s_data[1].substr(2, 2));
		time->tm_sec = stoi(s_data[1].substr(4, 2));

		this->time.tv_sec = mktime(time);

		if (s_data[1].length() > 6) // Subsecond precision
		{
			this->time.tv_usec = stoi(s_data[1].substr(7, 2))*10000;
		}
	}

	if (this->active)
	{
		lat = s_data[2].substr(0, 2);
		lat_dec = s_data[2].substr(2, s_data[2].length()-2);

		// Check if non empty. If so, update latitude
		if( ! lat.empty())
		{
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

		lon = s_data[4].substr(0, 3);
		lon_dec = s_data[4].substr(3, s_data[4].length()-3);

		// Check if non-empty. If so, update longitude
		if( ! lon.empty())
		{
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
		if( ! (s_data[7].empty()))
		{
			this->satellites = stoi(s_data[7]);
		}
		if( ! (s_data[8].empty()))
		{
			this->hdop = stof(s_data[8]);
		}
		if( ! (s_data[9].empty()))
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

	// Is the data valid?
	bool active = s_data[2] != "1";

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

	if (this->active)
	{
		// Update DOP
		if( ! (s_data[15].empty()))
		{
			this->pdop = stof(s_data[15]);
		}
		if( ! (s_data[16].empty()))
		{
			this->hdop = stof(s_data[16]);
		}
		if( ! (s_data[17].substr(0, s_data[17].find_first_of('*')).empty()))
		{
			this->vdop = stof(
				s_data[17].substr(0, s_data[17].find_first_of('*'))
			);
		}
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

	if (s_data[1].length() >= 6) // Update time
	{
		tm* time = gmtime(&this->time.tv_sec);
		time->tm_hour = stoi(s_data[1].substr(0, 2));
		time->tm_min = stoi(s_data[1].substr(2, 2));
		time->tm_sec = stoi(s_data[1].substr(4, 2));

		if (s_data[1].length() > 6) // Subsecond precision
		{
			this->time.tv_usec = stoi(s_data[1].substr(7, 2))*10000;
		}

		if (s_data[9].length() == 6) // Update date
		{
			time->tm_mday = stoi(s_data[9].substr(0, 2));
			time->tm_mon = stoi(s_data[9].substr(2, 2))-1;
			time->tm_year = stoi(s_data[9].substr(4, 2))+100;
		}

		this->time.tv_sec = mktime(time);
	}
	else if (s_data[9].length() == 6) // Update date
	{
		tm* time = gmtime(&this->time.tv_sec);

		time->tm_mday = stoi(s_data[9].substr(0, 2));
		time->tm_mon = stoi(s_data[9].substr(2, 2))-1;
		time->tm_year = stoi(s_data[9].substr(4, 2))+100;

		this->time.tv_sec = mktime(time);
	}

	if (this->active)
	{
		lat = s_data[3].substr(0, 2);
		lat_dec = s_data[3].substr(2, s_data[3].length()-2);
		// Check if non-empty, and if so, update latitude
		if( ! lat.empty())
		{
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

		lon = s_data[5].substr(0, 3);
		lon_dec = s_data[5].substr(3, s_data[5].length()-3);
		// Check if non-empty, and if so, update longitude
		if( ! lon.empty())
		{
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
		if( ! (s_data[7].empty()))
		{
			this->velocity.speed = kt_to_mps(stof(s_data[7]));
		}
		if( ! (s_data[8].empty()))
		{
			this->velocity.course = stof(s_data[8]);
		}
	}
}

void GPS::enter_airborne_1g_mode()
{
	int gps_dynamic_model_set_success = 0;
	struct timeval time_now, time_start;
	long ms_now, ms_start;

	unsigned char setdm6[] = // Byte at offset 2 determines new operation mode.
	{
		0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06,
		0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
		0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC
	};
	uint8_t sz_setdm6 = 44;

	gettimeofday(&time_start, NULL);
	ms_start = (long)((time_start.tv_sec)*1000 + (time_start.tv_usec)/1000);
	ms_now = ms_start;

	// Prevent lock and timeout if not set after six seconds
	while( ! gps_dynamic_model_set_success && (ms_now - ms_start)<6000)
	{
		gettimeofday(&time_now, NULL);
		ms_now = (long)((time_now.tv_sec)*1000 + (time_now.tv_usec)/1000);

		this->send_ublox_packet(setdm6, sz_setdm6);
		gps_dynamic_model_set_success = this->receive_check_ublox_ack(setdm6);
	}
	if (gps_dynamic_model_set_success)
	{
		this->logger->log("GPS entered airborne (<1g) mode successfully");
	}
	else
	{
		this->logger->log("GPS failed to enter airborne (<1g) mode");
	}
}

void GPS::enter_stationary_mode()
{
	int gps_dynamic_model_set_success = 0;
	struct timeval time_now, time_start;
	long ms_now, ms_start;

	unsigned char setdm2[] = // Byte at offset 2 determines new operation mode.
	{
		0xB5, 0x62, 0x02, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06,
		0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
		0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC
	};
	uint8_t sz_setdm2 = 44;

	gettimeofday(&time_start, NULL);
	ms_start = (long)((time_start.tv_sec)*1000 + (time_start.tv_usec)/1000);
	ms_now = ms_start;

	// Prevent lock and timeout if not set after six seconds
	while( ! gps_dynamic_model_set_success && (ms_now - ms_start)<6000)
	{
		gettimeofday(&time_now, NULL);
		ms_now = (long)((time_now.tv_sec)*1000 + (time_now.tv_usec)/1000);

		this->send_ublox_packet(setdm2, sz_setdm2);
		gps_dynamic_model_set_success = this->receive_check_ublox_ack(setdm2);
	}

	if (gps_dynamic_model_set_success)
	{
		this->logger->log("GPS entered stationary mode successfully");
	}
	else
	{
		this->logger->log("GPS failed to enter stationary mode");
	}
}

void GPS::enter_pedestrian_mode()
{
	int gps_dynamic_model_set_success = 0;
	struct timeval time_now, time_start;
	long ms_now, ms_start;

	unsigned char setdm3[] = // Byte at offset 2 determines new operation mode.
	{
		0xB5, 0x62, 0x03, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06,
		0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
		0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC
	};
	uint8_t sz_setdm3 = 44;

	gettimeofday(&time_start, NULL);
	ms_start = (long)((time_start.tv_sec)*1000 + (time_start.tv_usec)/1000);
	ms_now = ms_start;

	// Prevent lock and timeout if not set after six seconds
	while( ! gps_dynamic_model_set_success && (ms_now - ms_start)<6000)
	{
		gettimeofday(&time_now, NULL);
		ms_now = (long)((time_now.tv_sec)*1000 + (time_now.tv_usec)/1000);

		this->send_ublox_packet(setdm3, sz_setdm3);
		gps_dynamic_model_set_success = this->receive_check_ublox_ack(setdm3);
	}

	if (gps_dynamic_model_set_success)
	{
		this->logger->log("GPS entered pedestrian mode successfully");
	}
	else
	{
		this->logger->log("GPS failed to enter pedestrian mode");
	}
}

void GPS::send_ublox_packet(unsigned char *message, uint8_t len)
{
	if (this->serial->is_open())
	{
		this->serial->flush();
		this->serial->write((unsigned char) 0xFF);
		this_thread::sleep_for(500ms);
		for (unsigned int i = 0; i<len; i++)
		{
			this->serial->write(message[i]);
		}
	}
	else
	{
		this->logger->log("Tried to send a packet, but the serial is closed");
	}
}

bool GPS::receive_check_ublox_ack(unsigned char *message)
{
	unsigned char ack_packet[10];
	unsigned int bytes_ordered;
	unsigned char byte;
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

	for (unsigned int i = 0; i<8; i++)
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
		gettimeofday(&time_now, NULL);
		ms_now = (long)((time_now.tv_sec)*1000 + (time_now.tv_usec)/1000);

		if (bytes_ordered > 9)
		{
			return true;
		}

		if (this->serial->available())
		{
			byte = (unsigned char) this->serial->read_char();
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

void GPS::notify_initialization()
{
	this->logger->log("Initialization notified. Switching to pedestrian mode");
	this->enter_pedestrian_mode();
}

void GPS::notify_takeoff()
{
	this->logger->log("Takeoff notified. Switching to airborne mode");
	this->enter_airborne_1g_mode();
}

void GPS::notify_safe_mode()
{
	this->logger->log("Safe mode entry notified. Switching to airborne mode");
	this->enter_airborne_1g_mode();
}

void GPS::notify_landing()
{
	this->logger->log("Landing notified. Switching to stationary mode");
	this->enter_stationary_mode();
}

void GPS::update_date()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	this->logger->log("Current date - tv_sec: "+to_string(now.tv_sec)+" tv_usec: "+to_string(now.tv_usec));
	this->logger->log("New date - tv_sec: "+to_string(this->time.tv_sec)+" tv_usec: "+to_string(this->time.tv_usec));
	#ifndef OS_TESTING
		settimeofday(&this->time, NULL);
	#endif

	this->logger->log("Date changed.");
}
