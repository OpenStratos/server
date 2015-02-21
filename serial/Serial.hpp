#ifndef SERIAL_H
	#define SERIAL_H

	#include <functional>
	#include <cstdint>
	#include <string>
	#include <atomic>
	using namespace std;

	namespace os {
		class Serial
		{
		private:
			int fd;
			function<uint_fast8_t(const string&)> listener;
			atomic_bool open;
			atomic_bool stopped;
			string endl;

			void serial_thread();
		public:
			Serial() = default;
			Serial(Serial& copy) = delete;
			~Serial();

			uint_fast8_t send_frame(string frame);
			void close();
			bool is_valid(string frame);
			void initialize(const string& serial_URL, int baud, const string endl, function<uint_fast8_t(const string&)>);
		};
	}
#endif
