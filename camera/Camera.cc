#include "camera/Camera.h"

#include <cstdlib>
#include <cstdio>

#include <string>
#include <thread>
#include <chrono>

#include <sys/types.h>
#include <dirent.h>

#include "config.h"
#include "constants.h"

using namespace os;
using namespace std;

Camera& Camera::get_instance()
{
	static Camera instance;
	return instance;
}

Camera::~Camera()
{
	if (this->recording) this->stop();
}

void Camera::record_thread(int time)
{
	this_thread::sleep_for(chrono::milliseconds(time));
	this->recording = false;
}

bool Camera::record(int time)
{
	if ( ! this->recording)
	{
		string command;
		if (time > 0)
		{
			command = "raspivid -o data/video/test.h264 -t " + to_string(time) + " &";
		}
		else
		{
			command = "raspivid -o data/video/video-"+ to_string(get_file_count("data/video/"))
				+".h264 -t " + to_string(time) + " -w "+ to_string(VIDEO_WIDTH) +" -h "+ to_string(VIDEO_HEIGHT)
				+" -b "+ to_string(VIDEO_BITRATE*1000000) +" -fps "+ to_string(VIDEO_FPS) +" &";
		}
		#ifndef RASPIVID
			command = "";
		#endif

		int st = system(command.c_str());
		this->recording = true;

		if (time > 0)
		{
			thread t(&Camera::record_thread, this, time);
			t.detach();
		}

		return st == 0;
	}
}

bool Camera::record()
{
	return this->record(0);
}

bool Camera::stop()
{
	#ifdef RASPIVID
		if (system("pkill raspivid") == 0)
		{
			this->recording = false;
			return true;
		}
		return false;
	#else
		this->recording = false;
		return true;
	#endif
}

int os::get_file_count(const string& path)
{
	DIR *dp;
	int i = 0;
	struct dirent *ep;
	dp = opendir(path.c_str());

	while (ep = readdir(dp))
	{
		i++;
	}
	(void) closedir(dp);

	return i-2;
}
