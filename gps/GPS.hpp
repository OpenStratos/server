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

			time_t time;
			bool active;
			coordinate coord;
			double velocity;
			double angle;

			// TODO checksum, height
			static void parse(string frame);

			void setTime(time_t t);
			void setActive(bool active);
			void setCoordinate(coordinate c);
			void setVelocity(double v);
			void setAngle(double a);

			void serialPoll(int fd);

		public:
			GPS(string serialURL);
			~GPS(); //Close serial etc

			time_t getTime();
			bool isActive();
			coordinate getCoordinate();
			double getVelocity();
			double getAngle();
		};
	}
#endif