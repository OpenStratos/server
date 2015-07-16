#include <thread>
#include <iostream>

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
	cout << "Starting camera tests" << endl;
	#include "camera_test.cc"

	cout << "Starting serial tests" << endl;
	#include "serial_test.cc"

	cout << "Starting GPS tests" << endl;
	#include "gps_test.cc"

	cout << "Starting temperature tests" << endl;
	#include "temperature_test.cc"

	cout << "Starting battery tests" << endl;
	#include "battery_test.cc"
});
