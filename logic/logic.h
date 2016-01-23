#ifndef LOGIC_LOGIC_H_
#define LOGIC_LOGIC_H_

#include <thread>
#ifdef DEBUG
	#include <iostream>
#endif

#include <sys/time.h>
#ifndef NO_POWER_OFF
	#include <unistd.h>
	#include <sys/reboot.h>
#endif

#include <wiringPi.h>

#include "config.h"
#include "constants.h"
#include "utils.h"
#include "threads.h"
#include "logger/Logger.h"
#include "gps/GPS.h"
#include "camera/Camera.h"
#include "gsm/GSM.h"

namespace os
{
	void main_while(Logger* logger, State* state);
	void shut_down(Logger* logger);
}

#endif // LOGIC_LOGIC_H_
