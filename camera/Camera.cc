#include "camera/Camera.h"

#include <cstdlib>

#include <string>
#include <thread>
#include <chrono>

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

void Camera::record(int time)
{
	if ( ! this->recording)
	{
		string command = "raspivid -o os_video.h264 -t " + to_string(time) + " &";
		#ifndef RASPIVID
			command = "";
		#endif

		system(command.c_str());
		this->recording = true;

		if (time > 0)
		{
			thread t(&Camera::record_thread, this, time);
			t.detach();
		}
	}
}

void Camera::record()
{
	this->record(0);
}

void Camera::stop()
{
	system("pkill raspivid");
	this->recording = false;
}
