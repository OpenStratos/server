#include "Camera.hpp"

#include <cstdlib>
#include <string>

using namespace os;
using namespace std;

void Camera::record(int time)
{
	string command = "raspivid -o os_video.h264 -t " + to_string(time) + " &";
	system(command.c_str());
}

void Camera::record()
{
	this->record(0);
}

void Camera::stop()
{
	system("pkill raspivid");
}