#ifndef OPENSTRATOS_H_
#define OPENSTRATOS_H_

#include <cmath>

#include <iomanip>
#include <thread>

#include <sys/time.h>
#include <unistd.h>
#include <sys/reboot.h>

#include <wiringPi.h>

#include "config.h"
#include "constants.h"
#include "utils.h"
#include "threads.h"
#include "logger/Logger.h"
#include "gps/GPS.h"
#include "camera/Camera.h"
#include "gsm/GSM.h"

#if DEBUG
	#include <iostream>
#endif

namespace os
{
	void main_logic();
	void safe_mode();
	void main_while(Logger* logger, State* state);

	void initialize(Logger* logger, tm* now);
	void aquire_fix(Logger* logger);
	void start_recording(Logger* logger);
	void send_init_sms(Logger* logger);
	void wait_launch(Logger* logger);
	void go_up(Logger* logger);
	void go_down(Logger* logger);
	void land(Logger* logger);
	void shut_down(Logger* logger);

	inline bool has_launched(double launch_altitude);
	inline bool has_bursted(double maximum_altitude);
	inline bool has_landed();
}

using namespace std;
using namespace os;

#endif // OPENSTRATOS_H_
