#ifndef OPENSTRATOS_H_
#define OPENSTRATOS_H_

#include <thread>
#ifdef DEBUG
	#include <iostream>
#endif

#include <sys/time.h>
#ifndef NO_POWER_OFF
	#include <unistd.h>
	#include <sys/reboot.h>
#endif

#include "config.h"
#include "constants.h"
#include "utils.h"
#include "logic/logic.h"

using namespace std;
using namespace os;

#endif // OPENSTRATOS_H_
