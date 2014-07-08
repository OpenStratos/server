#ifndef CAMERA_HPP

#include <cstdlib>
#include <sstream>
#include <fstream>
#include <iterator>
#include <ctime>
#include <vector>

using namespace std;

namespace cam {

	unsigned char* get_static();
	unsigned char* get_static(int width, int height, int quality);
	unsigned char* get_static(int width, int height, int quality, int brightness, int contrast, int sharpness, int saturation, int iso);
	unsigned char* get_last_static();

	void start_video();
	void stop_video();
}

#define CAMERA_HPP
#endif