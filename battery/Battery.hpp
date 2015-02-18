#ifndef BATTERY_H
	#define BATTERY_H

	#include <atomic>
	#include "../constants.hpp"
	using namespace std;

	namespace os {
		class Battery
		{
		private:
			int address;
			int filehandle;
			float battery;
			atomic_bool reading;
			atomic_bool stopped;

			void read_battery();
		public:
			Battery(const int address);
			~Battery();
			Battery(Battery& copy) = delete;

			int get_battery() {return this->battery;}
			void start_reading();
			void stop_reading();
			bool is_reading() {return this->reading;}
		};

		inline float volt_to_percent(float voltage) {return (voltage-BAT_MIN)/(BAT_MAX-BAT_MIN)*100;}
	}
#endif
