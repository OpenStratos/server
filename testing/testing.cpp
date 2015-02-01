#define OS_TESTING

#include <bandit/bandit.h>
#include <thread>
#include <chrono>
#include <serial/Serial.hpp>
#include <camera/Camera.hpp>
#include <gps/GPS.hpp>
using namespace bandit;
using namespace os;
using namespace std;

int main(int argc, char* argv[])
{
	return run(argc, argv);
}

go_bandit([](){

	#include "core_test.cpp"
	#include "camera_test.cpp"
	#include "gps_test.cpp"
	#include "serial_test.cpp"
});
