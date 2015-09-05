#ifndef UTILS_H_
#define UTILS_H_

#include <string>

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
	inline bool file_exists(const string& name);
	inline float get_available_disk_space();

	State set_state(State new_state);
	State get_last_state();
	const string state_to_string(State state);
}

#endif // UTILS_H_
