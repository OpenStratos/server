#ifndef GPS_GPS_H_
#define GPS_GPS_H_

#include <cstdint>

#include <string>

#include "serial/Serial.h"
#include "logger/Logger.h"

using namespace std;

namespace os {

	struct euc_vec
	{
		float speed;
		float course;
	};

	class GPS
	{
	private:
		Serial serial;
		Logger* logger;
		Logger* frame_logger;

		tm time;
		bool active;
		uint_fast8_t satellites;
		double latitude;
		double longitude;
		double altitude;
		float pdop;
		float hdop;
		float vdop;
		euc_vec velocity;

		GPS() = default;

		void parse_GGA(const string& frame);
		void parse_GSA(const string& frame);
		void parse_RMC(const string& frame);

	public:
		GPS(GPS& copy) = delete;
		~GPS();
		static GPS& get_instance();
		static bool is_valid(string frame);

		tm get_time() const {return this->time;}
		bool is_active() const {return this->active;}
		uint_fast8_t get_satellites() const {return this->satellites;}
		double get_latitude() const {return this->latitude;}
		double get_longitude() const {return this->longitude;}
		double get_altitude() const {return this->altitude;}
		float get_PDOP() const {return this->pdop;}
		float get_HDOP() const {return this->hdop;}
		float get_VDOP() const {return this->vdop;}
		euc_vec get_velocity() const {return this->velocity;}

		bool initialize();
		bool turn_on() const;
		bool turn_off() const;
		void parse(const string& frame);
	};
}

inline float kt_to_mps(float knots) {return knots*463/900;}

#endif // GPS_GPS_H_
