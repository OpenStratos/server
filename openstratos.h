#ifndef OPENSTRATOS_H_
#define OPENSTRATOS_H_

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
