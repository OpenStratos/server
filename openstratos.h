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
		FIX_ACQUIRED,
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

	void initialize(Logger* logger, tm* now);
	void aquire_fix(Logger* logger);
	void start_recording(Logger* logger);
	void send_init_sms(Logger* logger);
	void wait_launch(Logger* logger);
	void go_up(Logger* logger);
	void go_down(Logger* logger);
	void land(Logger* logger);
	void shut_down(Logger* logger);

	inline bool has_launched();
	inline bool has_bursted();
	inline bool has_landed();

	void picture_thread_fn(State& state);
	void battery_thread_fn(State& state);

	void check_or_create(const string& path, Logger* logger = NULL);
	inline bool file_exists(const string& name);
	inline float get_available_disk_space();
	const string generate_exif_data();

	State set_state(State new_state);
	State get_last_state();
	const string state_to_string(State state);
}

using namespace std;
using namespace os;

#endif // OPENSTRATOS_H_
