#include "camera.hpp"

unsigned char* cam::get_static()
{
	system("raspstill -n -w 480 -h 270 -q 80 -o img/still.jpg");
	return get_last_static();
}

unsigned char* cam::get_static(int width, int height, int quality)
{
	if (width < 100 || width > 1920 || height < 50 || height > 1080 || quality < 50 || quality > 100)
		return nullptr;

	stringstream ss;
	ss << "raspstill -n -w " << width << " -h " << height << " -q " << quality << " -o img/still.jpg";

	system(ss.str().c_str());
	return get_last_static();
}

unsigned char* cam::get_static(int width, int height, int quality, int brightness, int contrast, int sharpness, int saturation, int iso)
{
	if (width < 100 || width > 1920 || height < 50 || height > 1080 || quality < 50 || quality > 100 ||
		brightness < 0 || brightness > 100 || contrast < -100 || contrast > 100 || sharpness < -100 ||
		sharpness > 100 || saturation < -100 || saturation > 100 || iso < 100 || iso > 800)
		return nullptr;

	stringstream ss;
	ss << "raspstill -n -w " << width << " -h " << height << " -q " << quality << " -br " << brightness <<
		" -co " << contrast << " -sh " << sharpness << " -sa " << saturation << " -ISO " << iso << " -o img/still.jpg";

	system(ss.str().c_str());
	return get_last_static();
}

unsigned char* cam::get_last_static()
{
	// Define file stream object, and open the file
	ifstream file;
	file.open("test.txt", ifstream::binary);

	// Prepare iterator pairs to iterate the file content
	istream_iterator<unsigned char> begin(file), end;

	// Reading the file content using the iterator
	vector<unsigned char> buffer(begin,end);
	buffer.push_back('\0');

	return &buffer[0];
}

void cam::start_video()
{
	char buff[20];
	time_t now = time (0);
	strftime(buff, 20, "%Y-%m-%d.%H:%M:%S", localtime(&now));

	stringstream ss;
	ss << "raspivid -t -1 -o video/vid-" << buff << ".h264";

	system(ss.str().c_str());
}

void cam::stop_video()
{
	system("pkill raspivid");
}

int main(void)
{
	cam::start_video();
	return 0;
}