#ifndef OPENSTRATOS_H_
#define OPENSTRATOS_H_

#include <cstdlib>
#include <cmath>

#include <iomanip>
#include <fstream>
#include <string>
#include <thread>
#include <functional>

#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/reboot.h>

#include <wiringPi.h>

#include "config.h"
#include "constants.h"
#include "logger/Logger.h"
#include "gps/GPS.h"
#include "camera/Camera.h"
#include "gsm/GSM.h"

#if DEBUG
	#include <iostream>
#endif

namespace os
{
	enum State {
		INITIALIZING,
		ACQUIRING_FIX,
		WAITING_LAUNCH,
		GOING_UP,
		GOING_DOWN,
		LANDED,
		SHUT_DOWN,
		SAFE_MODE,
		RECOVERY,
	};

	void main_logic();
	void safe_mode();

	inline bool file_exists(const string& name);
	inline float get_available_disk_space();
	void gps_thread_fn(State& state);
	void picture_thread_fn(State& state);
	void battery_thread_fn(State& state);
	State set_state(State new_state);
	State get_state();
	const string state_to_string(State state);
	bool has_launched();
	bool has_bursted();
	bool has_landed();
	const string generate_exif_data();
}

using namespace std;
using namespace os;

#endif // OPENSTRATOS_H_
