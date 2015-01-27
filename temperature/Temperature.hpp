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
			int lastTemp;
			bool reading;
			thread readThread;

			Temperature() = default;
			~Temperature();

			void startReading();
			void stopReading();
			void readTemperature();

		public:
			static Temperature& getInstance();

			int getLastTemp() {return this->lastTemp;}

			void initialize(const int devId);
		};
	}
	

#endif