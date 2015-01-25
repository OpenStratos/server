#ifndef GPS_H
	#define GPS_H

	#include <string>
	#include <thread>
	#include <cstdint>
	#include "../serial/Serial.hpp"

	using namespace std;

	namespace os {
		class GPS
		{
		private:
			Serial serial;

			time_t time;
			bool active;
			uint_fast8_t satellites;
			double latitude;
			double longitude;
			float altitude;
			float hdop;
			float vdop;

			GPS() = default;
			~GPS();

			void set_time(time_t time) {this->time = time;}
			void set_active(bool active) {this->active = active;}
			void set_satellites(uint_fast8_t satellites) {this->satellites = satellites;}
			void set_latitude(double latitude) {this->latitude = latitude;}
			void set_longitude(double longitude) {this->longitude = longitude;}
			void set_altitude(float altitude) {this->altitude = altitude;}
			void set_HDOP(float hdop) {this->hdop = hdop;}
			void set_VDOP(float vdop) {this->vdop = vdop;}

			void parse_GGA(const string& frame);
			void parse_GSA(const string& frame);
			void parse_RMC(const string& frame);

		public:
			static GPS& getInstance();

			time_t get_time() {return this->time;}
			bool is_active() {return this->active;}
			uint_fast8_t get_satellites() {return this->satellites;}
			double get_latitude() {return this->latitude;}
			double get_longitude() {return this->longitude;}
			float get_lltitude() {return this->altitude;}
			float get_HDOP() {return this->hdop;}
			float get_VDOP() {return this->vdop;}

			void initialize(const string& serial_URL);
			uint_fast8_t parse(const string& frame);
		};
	}
#endif
