#include "camera/Camera.h"

#include <cstdlib>
#include <cstdio>

#include <string>
#include <thread>
#include <chrono>

#include <sys/types.h>
#include <dirent.h>
#include <sys/time.h>

#include "config.h"
#include "constants.h"
#include "gps/GPS.h"

using namespace os;
using namespace std;

Camera& Camera::get_instance()
{
	static Camera instance;
	return instance;
}

Camera::Camera()
{
	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm * now = gmtime(&timer.tv_sec);

	this->logger = new Logger("data/logs/camera/Camera."+ to_string(now->tm_year+1900) +"-"+ to_string(now->tm_mon) +"-"+
		to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+ to_string(now->tm_min) +"-"+
		to_string(now->tm_sec) +".log", "Camera");
}

Camera::~Camera()
{
	this->logger->log("Shuting down...");
	if (this->recording)
	{
		this->logger->log("Stopping video recording...");
		if ( ! this->stop())
			this->logger->log("Error soping video recording.");
		else
			this->logger->log("Video recording stopped.");
	}
	this->logger->log("Shut down finished");
	delete this->logger;
}

void Camera::record_thread(int time)
{
	this->logger->log("Recording thread started.");
	this_thread::sleep_for(chrono::milliseconds(time));
	this->recording = false;
	this->logger->log("Finished recording thread.");
}

bool Camera::record(int time)
{
	if (time != 0) this->logger->log("Recording for "+to_string(time/1000)+" seconds...");
	if ( ! this->recording)
	{
		this->logger->log("Not already recording, creating command...");
		string filename = time > 0 ? "data/video/test.h264" : "data/video/video-"+
			to_string(get_file_count("data/video/")) +".h264";
		#ifdef OS_TESTING
			filename = "data/video/test.h264";
		#endif
		string command = "raspivid -n -o "+ filename +" -t " + to_string(time) + " -w "+ to_string(VIDEO_WIDTH) +" -h "
			+ to_string(VIDEO_HEIGHT) +" -b "+ to_string(VIDEO_BITRATE*1000000)
			+ " -fps "+ to_string(VIDEO_FPS) +" -co "+ to_string(VIDEO_CONTRAST)
			+ " -ex "+ VIDEO_EXPOSURE +" -br "+ to_string(VIDEO_BRIGHTNESS) +" &";
		this->logger->log("Video command: '"+command+"'");

		#ifndef RASPICAM
			this->logger->log("Test mode, video recording simulated.");
			command = "";
		#endif

		int st = system(command.c_str());
		this->recording = true;

		bool result = st == 0;

		if (result && time > 0)
		{
			this->logger->log("Starting recording thread...");
			thread t(&Camera::record_thread, this, time);
			t.detach();
		}

		if (result) this->logger->log("Video recording correctly started.");
		else this->logger->log("Error starting video recording.");

		return result;
	}
}

bool Camera::record()
{
	this->logger->log("Recording indefinitely...");
	return this->record(0);
}

bool Camera::take_picture(const string& exif)
{
	bool was_recording = this->recording;
	if (was_recording) this->logger->log("Recording video, stopping...");
	if (was_recording && ! this->stop()) return false;
	this->logger->log("Video recording stopped.");

	string filename = "data/img/img-"+ to_string(get_file_count("data/img/")) +".jpg";
	#ifdef OS_TESTING
		filename = "data/img/test.jpg";
	#endif

	string exif_command = exif != "" ? " -x "+ exif : "";

	string command = "raspistill -n -o "+ filename +" " + (PHOTO_RAW ? "-r" : "") + " -w "+ to_string(PHOTO_WIDTH)
				+" -h "+ to_string(PHOTO_HEIGHT) +" -q "+ to_string(PHOTO_QUALITY)
				+" -co "+ to_string(PHOTO_CONTRAST) +" -br "+ to_string(PHOTO_BRIGHTNESS)
				+" -ex "+ PHOTO_EXPOSURE + exif_command;

	this->logger->log("Picture command: '"+command+"'");

	#ifndef RASPICAM
		this->logger->log("Test mode, picture taking simulated.");
		command = "";
	#endif

	int st = system(command.c_str());
	bool result = st == 0;

	if (result) this->logger->log("Picture taken correctly.");
	else this->logger->log("Error taking picture.");

	if (was_recording) this->logger->log("Video recording was active before taking picture. Resuming...");
	if (was_recording && ! this->record())
	{
		this->logger->log("Error resuming video recording.");
		return false;
	}
	this->logger->log("Video recording resumed.");

	return result;
}

bool Camera::take_picture()
{
	return Camera::take_picture("");
}

bool Camera::stop()
{
	this->logger->log("Stopping video recording...");
	#ifdef RASPICAM
		if (system("pkill raspivid") == 0)
		{
			this->logger->log("Video recording stopped correctly.");
			this->recording = false;
			return true;
		}
		this->logger->log("Error stopping video recording.");

		if ( ! this->is_really_recording())
		{
			this->logger->log("Warning: video was already stopped.");
			this->recording = false;
			return true;
		}
		return false;
	#else
		this->logger->log("Test mode. Video recording stop simulated.");
		this->recording = false;
		return true;
	#endif
}

bool Camera::is_really_recording() const
{
	FILE* process = popen("pgrep raspivid", "r");
	char response[5];
	fgets(response, 5, process);
	pclose(process);

	if (process == NULL) this->logger->log("Error checking if raspivid is really recording");
	return (process != NULL && response != NULL);
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

const string os::generate_exif_data()
{
	string exif;
	while (GPS::get_instance().get_PDOP() > 5)
	{
		this_thread::sleep_for(1s);
	}
	double gps_lat = GPS::get_instance().get_latitude();
	double gps_lon = GPS::get_instance().get_longitude();
	double gps_alt = GPS::get_instance().get_altitude();
	uint_fast8_t gps_sat = GPS::get_instance().get_satellites();
	float gps_pdop = GPS::get_instance().get_PDOP();
	euc_vec gps_velocity = GPS::get_instance().get_velocity();

	exif += "GPSLatitudeRef="+to_string(gps_lat > 0 ? 'N' : 'S');
	exif += " GPSLatitude="+to_string(abs((int) gps_lat*1000000))+"/1000000,0/1,0/1";
	exif += " GPSLongitudeRef="+to_string(gps_lon > 0 ? 'E' : 'W');
	exif += " GPSLongitude="+to_string(abs((int) gps_lon*1000000))+"/1000000,0/1,0/1";
	exif += " GPSAltitudeRef=0 GPSAltitude="+to_string(gps_alt);
	exif += " GPSSatellites="+to_string(gps_sat);
	exif += " GPSDOP="+to_string(gps_pdop);
	exif += " GPSSpeedRef=K GPSSpeed="+to_string(gps_velocity.speed*3.6);
	exif += " GPSTrackRef=T GPSTrack="+to_string(gps_velocity.course);
	exif += " GPSDifferential=0";
}
