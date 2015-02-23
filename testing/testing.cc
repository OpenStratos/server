#include <thread>
#include <chrono>

#include <bandit/bandit.h>

#include "serial/Serial.h"
#include "camera/Camera.h"
#include "temperature/Temperature.h"
#include "gps/GPS.h"
#include "battery/Battery.h"

using namespace bandit;
using namespace os;
using namespace std;


int main(int argc, char* argv[])
{
	return run(argc, argv);
}

go_bandit([](){

	#include "core_test.cc"
	#include "camera_test.cc"
	#include "serial_test.cc"
	#include "gps_test.cc"
	#include "temperature_test.cc"
	#include "battery_test.cc"
});
