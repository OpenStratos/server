#ifndef GPS_H
	#define GPS_H

	#include <ctime>
	#include <string>
	using namespace std;

	namespace gps {

		class GPS: public IGPS
		{
		private:
			string raw_frame;
			time_t time;
			bool isActive;
			coordinate coord;
			double velocity;
			double angle;

			// TODO checksum, height

			GPS();
			void setTime(time_t t);
			void setActive(bool active);
			void setCoordinate(coordinate c);
			void setVelocity(double v);
			void setAngle(double a);

		public:
			static GPS parse(string frame);

			time_t getTime();
			bool isActive();
			coordinate getCoordinate();
			double getVelocity();
			double getAngle();
		};
	}
#ifndef