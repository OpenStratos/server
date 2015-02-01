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
			bool get_reading() {return this->reading;}

		};
	}
	
	inline float r_to_c(float r) 
	{ 
		float value = r - 1000;
		return (value / 3.91 + value * value / 100000); 
	}

#endif
	