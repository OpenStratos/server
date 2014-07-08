#include "camera.hpp"

unsigned char* cam::get_static()
{
	system("raspstill -n -w 480 -h 270 -q 80 -o img/still.jpg");
	return get_last_static();
}

unsigned char* cam::get_static(int width, int height, int quality)
{
	if (width < 100 || width > 1920 || height < 50 || height > 1080 || quality < 50 || quality > 100)
		return null;

	system("raspstill -n -w "+ width +" -h "+ height +" -q "+ quality +" -o img/still.jpg");
	return get_last_static();
}

unsigned char* cam::get_static(int width, int height, int quality, int brightness, int contrast, int sharpness, int saturation, int iso)
{
	if (width < 100 || width > 1920 || height < 50 || height > 1080 || quality < 50 || quality > 100 ||
		brightness < 0 || brightness > 100 || contrast < -100 || contrast > 100 || sharpness < -100 ||
		sharpness > 100 || saturation < -100 || saturation > 100 || iso < 100 || iso > 800)
		return null;
	system("raspstill -n -w "+ width +" -h "+ height +" -q "+ quality +" -br "+ brightness +
		" -co "+ contrast +" -sh "+ sharpness +" -sa "+ saturation +" -ISO "+ iso +" -o img/still.jpg");
	return get_last_static();
}

unsigned char* cam::get_last_static()
{
	// Define file stream object, and open the file
	ifstream file("img/still.jpg", ios::binary);

	// Prepare iterator pairs to iterate the file content!
	istream_iterator<unsigned char> begin(file), end;

	// Reading the file content using the iterator!
	vector<unsigned char> buffer(begin,end);
	buffer.push_back(null);

	return &buffer[0];
}

void cam::start_video()
{
	auto t = time(nullptr);
	auto tm = *localtime(&t);
	system("raspivid -t -1 -o video/vid-"+ put_time(&tm, "%d-%m-%Y %H-%M-%S") +".h264");
}
void cam::stop_video()
{
	system("pkill raspivid");
}