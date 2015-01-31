#ifndef TEMPERATURE_H
	#define TEMPERATURE_H

	using namespace std;

	namespace os {
		class Temperature
		{
		private:
			int devId;
			int filehandle;
			float lastTemp;
			bool reading;

			Temperature(const int devId);
			~Temperature();

			void read_temperature();

		public:
			int get_last_temp() {return this->lastTemp;}
			void start_reading();
			void stop_reading();

		};
	}
	
	inline float r_to_c(float r) { return ((r - 1000) / 3.91 + (r - 1000) * 2 / 100000); }

#endif