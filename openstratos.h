#ifndef OPENSTRATOS_H_
#define OPENSTRATOS_H_

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

#ifdef DEBUG
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
	void wait_launch(Logger* logger, double& launch_altitude);
	void go_up(Logger* logger, double launch_altitude);
	void go_down(Logger* logger);
	void land(Logger* logger);
	void shut_down(Logger* logger);
}

using namespace std;
using namespace os;

#endif // OPENSTRATOS_H_
