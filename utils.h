#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <thread>

#include <sys/stat.h>
#include <sys/statvfs.h>

#include "constants.h"
#include "logger/Logger.h"
#include "gps/GPS.h"

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

	inline State get_real_state()
	{
		double start_alt = 0;//GPS::get_instance().get_altitude();
		this_thread::sleep_for(5s);
		double end_alt = 0;//GPS::get_instance().get_altitude();

		if (end_alt - start_alt < -10) return set_state(GOING_DOWN);
		else if (end_alt - start_alt > 5) return set_state(GOING_UP);
		else if (end_alt > 8000) return set_state(GOING_DOWN);
		else return set_state(LANDED);
	}
}

#endif // UTILS_H_
