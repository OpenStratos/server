#ifndef GPS_H
	#define GPS_H

	#include <string>
	#include <cstdint>
	#include "../serial/Serial.hpp"

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

			time_t time;
			bool active;
			uint_fast8_t satellites;
			double latitude;
			double longitude;
			double altitude;
			float hdop;
			float vdop;
			euc_vec velocity;

			GPS() = default;
			~GPS();

			void parse_GGA(const string& frame);
			void parse_GSA(const string& frame);
			void parse_RMC(const string& frame);

		public:
			static GPS& get_instance();

			time_t get_time() {return this->time;}
			bool is_active() {return this->active;}
			uint_fast8_t get_satellites() {return this->satellites;}
			double get_latitude() {return this->latitude;}
			double get_longitude() {return this->longitude;}
			double get_altitude() {return this->altitude;}
			float get_HDOP() {return this->hdop;}
			float get_VDOP() {return this->vdop;}
			euc_vec get_velocity() {return this->velocity;}

			void initialize(const string& serial_URL);
			uint_fast8_t parse(const string& frame);
		};
	}

	inline float kt_to_mps(float knots) {return knots*463/900;}
#endif
