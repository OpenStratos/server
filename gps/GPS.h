#ifndef GPS_GPS_H_
#define GPS_GPS_H_

#include <cstdint>

#include <string>
#include <atomic>

#include <sys/time.h>

#include "serial/Serial.h"
#include "logger/Logger.h"

using namespace std;

namespace os {

	typedef struct
	{
		float speed;
		float course;
	} euc_vec;

	class GPS
	{
	private:
		Serial* serial;
		Logger* logger;
		Logger* frame_logger;

		atomic_bool should_stop;
		atomic_bool stopped;

		timeval time;
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

		void gps_thread();

		void parse_GGA(const string& frame);
		void parse_GSA(const string& frame);
		void parse_RMC(const string& frame);

		bool enter_airborne_1g_mode();
		void send_ublox_packet(const vector<uint8_t> &message);
		bool receive_check_ublox_ack(const vector<uint8_t> &message);

	public:
		GPS(GPS& copy) = delete;
		~GPS();
		static GPS& get_instance();
		static bool is_valid(string frame);

		timeval get_time() const {return this->time;}
		bool is_fixed() const {return this->active;}
		uint_fast8_t get_satellites() const {return this->satellites;}
		double get_latitude() const {return this->latitude;}
		double get_longitude() const {return this->longitude;}
		double get_altitude() const {return this->altitude;}
		float get_PDOP() const {return this->pdop;}
		float get_HDOP() const {return this->hdop;}
		float get_VDOP() const {return this->vdop;}
		euc_vec get_velocity() const {return this->velocity;}

		bool initialize();
		bool is_on() const;
		bool turn_on() const;
		bool turn_off() const;
		void parse(const string& frame);
	};
}

inline float kt_to_mps(float knots) {return knots*463/900;}

#endif // GPS_GPS_H_
