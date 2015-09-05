#ifndef UTILS_H_
#define UTILS_H_

#include <string>

#include <sys/stat.h>
#include <sys/statvfs.h>

#include "logger/Logger.h"

namespace os {
	enum State {
		INITIALIZING,
		ACQUIRING_FIX,
		FIX_ACQUIRED,
		WAITING_LAUNCH,
		GOING_UP,
		GOING_DOWN,
		LANDED,
		SHUT_DOWN,
		SAFE_MODE,
	};

	void check_or_create(const string& path, Logger* logger = NULL);

	inline bool file_exists(const string& name)
	{
		struct stat buffer;
		return stat(name.c_str(), &buffer) == 0;
	}

	inline float get_available_disk_space()
	{
		struct statvfs fs;
		statvfs("data", &fs);

		return ((float) fs.f_bsize)*fs.f_bavail;
	}

	State set_state(State new_state);
	State get_last_state();
	const string state_to_string(State state);
}

#endif // UTILS_H_
