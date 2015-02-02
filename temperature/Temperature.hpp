#ifndef TEMPERATURE_H
	#define TEMPERATURE_H

	using namespace std;

	namespace os {
		class Temperature
		{
		private:
			int address;
			int filehandle;
			float temperature;
			volatile bool reading;
			volatile bool stopped;

			void read_temperature();
		public:
			Temperature(const int address);
			~Temperature();
			Temperature(Temperature& copy) = delete;

			int get_temperature() {return this->temperature;}
			void start_reading();
			void stop_reading();
			bool is_reading() {return this->reading;}
		};
	}

	inline float r_to_c(float r)
	{
		float value = r - 1000;
		return (value / 3.91 + value * value / 100000);
	}
#endif
