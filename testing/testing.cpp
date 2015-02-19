#include <bandit/bandit.h>
#include <thread>
#include <chrono>
#include <serial/Serial.hpp>
#include <camera/Camera.hpp>
#include <temperature/Temperature.hpp>
#include <gps/GPS.hpp>
#include <battery/Battery.hpp>
#include <iostream>

using namespace bandit;
using namespace os;
using namespace std;


int main(int argc, char* argv[])
{
	return run(argc, argv);
}

go_bandit([](){

	cout << "Starting core test" << endl;
	#include "core_test.cpp"

	cout << "Starting camera test" << endl;
	#include "camera_test.cpp"

	cout << "Starting Serial test" << endl;
	#include "serial_test.cpp"

	cout << "Starting GPS test" << endl;
	#include "gps_test.cpp"

	cout << "Starting Temperature test" << endl;
	#include "temperature_test.cpp"

	cout << "Starting Battery test" << endl;
	#include "battery_test.cpp"
});
