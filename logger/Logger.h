#ifndef LOGGER_LOGGER_H_
#define LOGGER_LOGGER_H_

#include <string>
#include <fstream>

using namespace std;

namespace os {

	class Logger
	{
	private:
		ofstream log_stream;
		string log_prefix;
	public:
		Logger() = delete;
		Logger(Logger& copy) = delete;
		~Logger();

		Logger(const string& path, const string& prefix);
		void log(const string& message);
	};
}

#endif // LOGGER_LOGGER_H_
