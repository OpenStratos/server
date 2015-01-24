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

			void setTime(time_t time) {this->time = time;}
			void setActive(bool active) {this->active = active;}
			void setSatellites(uint_fast8_t satellites) {this->satellites = satellites;}
			void setLatitude(double latitude) {this->latitude = latitude;}
			void setLongitude(double longitude) {this->longitude = longitude;}
			void setAltitude(float altitude) {this->altitude = altitude;}
			void setHDOP(float hdop) {this->hdop = hdop;}
			void setVDOP(float vdop) {this->vdop = vdop;}

			void serialPoll();
			void parse(string frame);

		public:
			static GPS& getInstance();

			time_t getTime() {return this->time;}
			bool getActive() {return this->active;}
			uint_fast8_t getSatellites() {return this->satellites;}
			double getLatitude() {return this->latitude;}
			double getLongitude() {return this->longitude;}
			float getAltitude() {return this->altitude;}
			float getHDOP() {return this->hdop;}
			float getVDOP() {return this->vdop;}

			void initialize(Serial serial);
		};
	}
#endif