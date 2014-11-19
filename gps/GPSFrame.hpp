#ifndef GPSFRAME_H
	#define GPSFRAME_H

	#include <ctime>
	#include <string>

	using namespace std;

	namespace gps {

		struct coordinate {
			double latitude;
			double longitude;
		};

		class GPSFrame
		{
		private:
			string raw_frame;
			time_t time;
			bool is_valid;
			coordinate coord;
			double velocity;
			double angle;

			// TODO checksum, height

			GPSFrame();
			void setTime(time_t t);
			void setValid(bool valid);
			void setCoordinate(coordinate c);
			void setVelocity(double v);
			void setAngle(double a);

		public:
			static GPSFrame parse(string frame);

			time_t getTime();
			bool isValid();
			coordinate getCoordinate();
			double getVelocity();
			double getAngle();
		};
	}
#ifndef