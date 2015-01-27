#ifndef TEMPERATURE_H
	#define TEMPERATURE_H
	#include <wiringPiI2C.h>
	#include <string>
	#include <thread>
	#include <chrono>

	using namespace std;

	namespace os {
		class Temperature
		{
		private:
			int devId;
			int filehandle;
			float lastTemp;
			bool reading;

			Temperature() = default;
			~Temperature();

			void read_temperature();

		public:
			static Temperature& get_instance();

			int get_last_temp() {return this->lastTemp;}
			void start_reading();
			void stop_reading();

			void initialize(const int devId);
		};
	}
	
	inline float r_to_c(float r) { return (r - 1000 / 3.91);}

#endif