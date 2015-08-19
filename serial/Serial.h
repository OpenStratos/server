#ifndef SERIAL_SERIAL_H_
#define SERIAL_SERIAL_H_

#include <cstdint>

#include <functional>
#include <string>
#include <atomic>
using namespace std;

namespace os {
	class Serial
	{
	private:
		int fd;
		atomic_bool open;
		atomic_bool stopped;

		void gps_thread();
	public:
		Serial() = default;
		Serial(Serial& copy) = delete;
		~Serial();

		void println(const string& str) const;
		void println() const;
		void write(const string& str) const;
		void close();
		bool is_open();
		bool initialize_GPS();
		bool initialize(const string& serial_URL, int baud);
		const string read_line() const;
		const string read_line(double timeout) const;
		bool read_only(const string& only) const;
		void flush() const;
	};
}

#endif // SERIAL_SERIAL_H_
