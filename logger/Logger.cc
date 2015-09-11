#include "logger/Logger.h"

#include <string>
#include <iomanip>

#include <sys/time.h>

using namespace std;
using namespace os;

Logger::Logger(const string& path, const string& prefix)
{
	this->log_stream.open(path);
	this->log_prefix = prefix;
	this->log("Logging started.");
}

Logger::~Logger()
{
	this->log_stream.close();
}

void Logger::log(const string& message)
{
	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm * now = gmtime(&timer.tv_sec);

	this->log_stream << "[" << log_prefix << "] - "  << setfill('0') << setw(2) << now->tm_mon << "/" <<
		setfill('0') << setw(2) << now->tm_mday << "/" << (now->tm_year+1900) << " " <<
		setfill('0') << setw(2) << now->tm_hour << ":" << setfill('0') << setw(2) << now->tm_min << ":" <<
		setfill('0') << setw(2) << now->tm_sec << "." << setfill('0') << setw(6) << timer.tv_usec <<
		" - " << message << endl;
}
