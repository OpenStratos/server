#ifndef IGPS_H
	#define IGPS_H

	#include "GPS.hpp"
	using namespace std;

	namespace gps {

		struct coordinate {
			double latitude;
			double longitude;
		};

		class IGPS
		{
		public:
			static IGPS start(string serialURL)
			{
				return new GPS(serialURL);
			}

		};
	}
#ifndef