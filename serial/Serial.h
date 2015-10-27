#ifndef SERIAL_SERIAL_H_
#define SERIAL_SERIAL_H_

#include <cstdint>

#include <string>

#include "constants.h"
#ifdef DEBUG
	#include "logger/Logger.h"
#endif
using namespace std;

namespace os {
	class Serial
	{
	private:
		int fd;
		bool open;

		#ifdef DEBUG
			Logger* logger;
		#endif

		void gps_thread();
	public:
		Serial(const string& url, int baud_rate, const string& log_path);
		Serial(Serial& copy) = delete;
		~Serial();

		void println(const string& str) const;
		void println() const;
		void write(unsigned char c) const;
		void close();
		bool is_open() const;
		char read_char() const;
		int available() const;
		const string read_line() const;
		const string read_line(double timeout) const;
		bool read_only(const string& only) const;
		void flush() const;
	};
}

#endif // SERIAL_SERIAL_H_
