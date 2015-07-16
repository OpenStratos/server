#include "openstratos.h"

int main(void)
{
	cout << "[OpenStratos] Starting..." << endl; // Only if verbose

	struct timeval timer;
	struct timezone tz;

	gettimeofday(&timer, &tz);

	struct tm * now = gmtime(&timer.tv_sec);

	cout << "[OpenStratos] Current time: " << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec <<
		" UTC of " << now->tm_mon << "/" << now->tm_mday << "/" << (now->tm_year+1900) << endl; // Only if verbose

	if ( ! file_exists("data"))
	{
		cout << "[OpenStratos] No data directory, creating..." << endl;
		if (mkdir("data", 0755) == 0)
		{
			cout << "[OpenStratos] Data directory created." << endl;
		}
		else
		{
			cout << "[OpenStratos] Error creating data directory." << endl;
			exit(1);
		}
	}

	if ( ! file_exists("data/logs"))
	{
		cout << "[OpenStratos] No log directory, creating..." << endl;
		if (mkdir("data/logs", 0755) == 0 && mkdir("data/logs/main", 0755) == 0 &&
			mkdir("data/logs/camera", 0755) == 0 && mkdir("data/logs/gps", 0755) == 0)
		{
			cout << "[OpenStratos] Log directory created." << endl;
		}
		else
		{
			cout << "[OpenStratos] Error creating log directory." << endl;
			exit(1);
		}
	}

	cout << "[OpenStratos] Starting logger..." << endl; // Only if verbose

	Logger logger("data/logs/main/OpenStratos."+ to_string(now->tm_year+1900) +"-"+ to_string(now->tm_mon) +"-"+
		to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+ to_string(now->tm_min) +"-"+
		to_string(now->tm_sec) +".log", "OpenStratos");
	cout << "[OpenStratos] Logger started." << endl; // Only if verbose

	if (file_exists("data/video"))
	{
		logger.log("Video directory exists.");
	}
	else
	{
		logger.log("No video directory, creating...");
		if (mkdir("data/video", 0755) == 0)
		{
			logger.log("Video directory created.");
		}
		else
		{
			logger.log("Error creating video directory.");
			cout << "[OpenStratos] Error creating video directory." << endl;
			exit(1);
		}
	}

	logger.log("Available disk space: " + to_string(get_available_disk_space()/1073741824) + " GiB");
	if (get_available_disk_space() < 21705523200) // Enough for about 3 hours of video
	{
		logger.log("Error: Not enough disk space.");
		cout << "[OpenStratos] Error: Not enough disk space." << endl;
		exit(1);
	}

	// 115 MiB per minute +/-
	logger.log("Disk space enough for about " + to_string(get_available_disk_space()/7235174400) +
		" hours of fullHD video.");

	// logger.log("Turning on GPS...");
	// if ( ! GPS::get_instance().initialize(""))
	// {
	// 	logger.log("GPS initialization error.");
	// 	exit(1);
	// }
	// logger.log("GPS On.");

	// logger.log("Starting GPS Thread...");
	// thread gps_thread(gps_thread_fn);
	// logger.log("GPS thread started.");

	logger.log("Starting camera recording...");
	if ( ! RASPIVID)
	{
		logger.log("Error: No raspivid found. Is this a Raspberry?");
		exit(1);
	}
	logger.log("Recording 10 seconds as test...");
	Camera::get_instance().record(10000);
	this_thread::sleep_for(11s);
	Camera::get_instance().stop();

	if (file_exists("data/video/test.h264"))
	{
		logger.log("Camera test OK.");
		logger.log("Removing test file...");
		if (remove("data/video/test.h264"))
		{
			logger.log("Error removing test file.");
		}
		else
		{
			logger.log("Test file removed.");
		}
	}
	else
	{
		logger.log("Test recording error.");
		exit(1);
	}

	logger.log("Starting video recording...");
	Camera::get_instance().record();

	logger.log("Stopping video...");
	Camera::get_instance().stop();

	logger.log("Joining threads...");
	// gps_thread.join();
	logger.log("Finishing execution.");
	return 0;
}

inline bool file_exists(const string& name)
{
	struct stat buffer;
	return (stat (name.c_str(), &buffer) == 0);
}

inline float get_available_disk_space()
{
	struct statvfs fs;
	statvfs("data", &fs);

	return fs.f_bsize*fs.f_bavail;
}

void gps_thread_fn()
{
	while ( ! GPS::get_instance().is_active())
	{
		this_thread::sleep_for(1s);
	}
	this_thread::sleep_for(2s);

	struct timezone tz = {0, 0};
	struct timeval tv = {timegm(GPS::get_instance().get_time()), 0};

	settimeofday(&tv, &tz);

	// TODO
}
