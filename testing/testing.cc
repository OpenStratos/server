#include <thread>

#include <sys/stat.h>

#include <bandit/bandit.h>

#include "config.h"
#include "constants.h"

#include "camera/Camera.h"
#include "gps/GPS.h"

using namespace bandit;
using namespace os;
using namespace std;

inline bool file_exists(const string& name);

int main(int argc, char* argv[])
{
	return run(argc, argv);
}

go_bandit([](){
	if ( ! file_exists("data"))
		mkdir("data", 0755);

	if ( ! file_exists("data/logs"))
	{
		mkdir("data/logs", 0755);
		mkdir("data/logs/main", 0755);
		mkdir("data/logs/camera", 0755);
		mkdir("data/logs/GPS", 0755);
		mkdir("data/logs/GSM", 0755);
	}

	if ( ! file_exists("data/video"))
		mkdir("data/video", 0755);

	if ( ! file_exists("data/img"))
		mkdir("data/img", 0755);

	#include "camera_test.cc"
	#include "gps_test.cc"
});

inline bool file_exists(const string& name)
{
	struct stat buffer;
	return stat(name.c_str(), &buffer) == 0;
}
