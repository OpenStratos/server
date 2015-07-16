#include "logger/Logger.h"

#include <string>
#include <iostream>

#include <sys/time.h>

using namespace std;
using namespace os;

Logger::Logger(const string& path, const string& prefix)
{
	this->log_stream.open(path);
	this->log_prefix = prefix;
	this->log("Logging started");
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

	this->log_stream << "[" << log_prefix << "] - "  << now->tm_mon << "/" << now->tm_mday << "/" << (now->tm_year+1900)
		<< " " << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec << "." << timer.tv_usec << " "
		<< message << endl;
}
