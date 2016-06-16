#ifndef OPENSTRATOS_H_
#define OPENSTRATOS_H_

#if defined SIM && defined REAL_SIM
	#error You cannot try to compile a real simulation and a normal simulation at the same time.
#endif

#ifdef DEBUG
	#include <iostream>
#endif

#ifndef NO_POWER_OFF
	#include <unistd.h>
	#include <sys/reboot.h>
#endif

#include "config.h"
#include "utils.h"
#include "logic/logic.h"

using namespace std;
using namespace os;

#endif // OPENSTRATOS_H_
