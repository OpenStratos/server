#ifndef GPS_H
	#define GPS_H

	#include <ctime>
	#include <string>
	#include <thread>

	using namespace std;

	namespace os {
		struct coordinate {
			double latitude;
			double longitude;
		};

		class GPS
		{
		private:
			int fd;
			thread gpsThread;

			bool active;
			coordinate coord;
			float velocity;
			float angle;
			float altitude;

			// TODO checksum, height

			void serialPoll();
			void parse(string frame);

			void setActive(bool active);
			void setCoordinate(coordinate c);
			void setVelocity(float v);
			void setAngle(float a);
			void setAltitude(float a);

		public:
			GPS(string serialURL);
			~GPS(); // Close serial etc

			bool isActive() const;
			coordinate getCoordinate() const;
			float getVelocity() const;
			float getAngle() const;
			float getAltitude() const;
		};
	}
#endif