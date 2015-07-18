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
		function<uint_fast8_t(const string&)> listener;
		atomic_bool open;
		atomic_bool stopped;
		string endl;

		void serial_thread();
	public:
		Serial() = default;
		Serial(Serial& copy) = delete;
		~Serial();

		void send(const string& str) const;
		void close();
		void initialize(const string& serial_URL, int baud, const string endl, function<uint_fast8_t(const string&)>);
		bool initialize(const string& serial_URL, int baud);
		const string read_line() const;
		void flush() const;
	};
}

#endif // SERIAL_SERIAL_H_
