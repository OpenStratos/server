#ifndef SERIAL_H
	#define SERIAL_H

	#include <functional>
	#include <cstdint>
	#include <string>
	using namespace std;

	namespace os {
		class Serial
		{
		private:
			int fd;
			function<uint_fast8_t(const string&)> listener;
			volatile bool open;
			volatile bool stopped;
			string endl;

			void serial_thread();
		public:
			Serial(const string& serial_URL, int baud, const string endl, function<uint_fast8_t(const string&)>);
			Serial() = default;
			Serial(Serial& copy) = delete;
			~Serial();

			uint_fast8_t send_frame(string frame);
			void close();
			bool is_valid(string frame);
		};
	}
#endif
