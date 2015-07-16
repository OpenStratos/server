#include "camera/Camera.h"

#include <cstdlib>
#include <cstdio>

#include <string>
#include <thread>
#include <chrono>

#include <iostream>

#include <sys/types.h>
#include <dirent.h>

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
	cout << "record called" << endl;
	cout << "is recording?: " << this->recording << endl;
	if ( ! this->recording)
	{
		cout << "Not recording, so creating command" << endl;
		string command;
		if (time > 0)
		{
			cout << "Command with time." << endl;
			command = "raspivid -o data/video/test.h264 -t " + to_string(time) + " &";
		}
		else
		{
			cout << "Command without time." << endl;
			command = "raspivid -o data/video/video-"+ to_string(get_file_count("data/video/"))
				+".h264 -t " + to_string(time) + " &";
			cout << "Command created." << endl;
			cout << "Command: '" << command << "'" << endl;
		}
		#ifndef RASPIVID
			command = "";
		#endif

		cout << "Command: '" << command << "'" << endl;

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
