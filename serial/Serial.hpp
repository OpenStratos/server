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
			bool open;
			string endl;

			void serial_thread();
		public:
			Serial(const string& serial_URL, int baud, const string endl, function<uint_fast8_t(const string&)>);
			~Serial();

			uint_fast8_t send_frame(string frame);
			void close();
			bool is_valid(string frame);
		};
	}
#endif
