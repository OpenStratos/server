#include "openstratos.h"

int main(void)
{
	cout << "[OpenStratos] Starting..." << endl; // Only if verbose

	State state = INITIALIZING;

	struct timeval timer;
	gettimeofday(&timer, NULL);
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
			mkdir("data/logs/camera", 0755) == 0 && mkdir("data/logs/GPS", 0755) == 0)
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

	// ~115 MiB per minute
	logger.log("Disk space enough for about " + to_string(get_available_disk_space()/7235174400) +
		" hours of fullHD video.");

	// logger.log("Turning on GPS...");
	// if ( ! GPS::get_instance().initialize(""))
	// {
	// 	logger.log("GPS initialization error.");
	// 	exit(1);
	// }
	// logger.log("GPS On.");

	// TODO start GSM and send message

	logger.log("Starting camera recording...");
	#ifndef RASPIVID
		logger.log("Error: No raspivid found. Is this a Raspberry?");
		exit(1);
	#endif
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

	state = set_state(ACQUIRING_FIX);
	logger.log("State changed to "+ state_to_string(state) +".");
	while ( ! GPS::get_instance().is_active())
	{
		this_thread::sleep_for(1s);
	}
	logger.log("GPS fix acquired, waiting for date change.");
	this_thread::sleep_for(2s);

	struct timezone tz = {0, 0};
	tm gps_time = GPS::get_instance().get_time();
	struct timeval tv = {timegm(&gps_time), 0};
	settimeofday(&tv, &tz);

	logger.log("System date change.");

	logger.log("Starting GPS Thread...");
	thread gps_thread(&gps_thread_fn, ref(state));
	logger.log("GPS thread started.");

	// TODO send SMS

	state = set_state(WAITING_LAUNCH);
	logger.log("State changed to "+ state_to_string(state) +".");
	logger.log("Waiting for launch...");

	// Main logic
	while (state != SHUT_DOWN)
	{
		switch (state)
		{
			case WAITING_LAUNCH:
				// TODO detect launch and send SMS
			break;
			case GOING_UP:
				// TODO detect records and check recording
			break;
			case GOING_DOWN:
				// TODO detect 3 km mark and send SMS
				// TODO detect 1,5 km mark and send SMS
				// TODO detect 500m mark and send SMS if not landed
				// TODO detect landing
			break;
			case LANDED:
				// TODO send SMS with position after 1 minute
				// TODO send SMS with position after 15 minutes and shut down.
				break;
		}
	}

	logger.log("Stopping video...");
	Camera::get_instance().stop();

	logger.log("Joining threads...");
	// gps_thread.join();
	logger.log("Finishing execution.");
	return 0;
}

inline bool os::file_exists(const string& name)
{
	struct stat buffer;
	return (stat (name.c_str(), &buffer) == 0);
}

inline float os::get_available_disk_space()
{
	struct statvfs fs;
	statvfs("data", &fs);

	return fs.f_bsize*fs.f_bavail;
}

void os::gps_thread_fn(State& state)
{
	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm * now = gmtime(&timer.tv_sec);

	Logger logger("data/logs/GPS/Position."+ to_string(now->tm_year+1900) +"-"+ to_string(now->tm_mon) +"-"+
		to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+ to_string(now->tm_min) +"-"+
		to_string(now->tm_sec) +".log", "GPSPosition");

	while (state != LANDED) {
		logger.log("Lat: "+ to_string(GPS::get_instance().get_latitude()) +", Lon: "+
			to_string(GPS::get_instance().get_longitude()) +", Alt: "+
			to_string(GPS::get_instance().get_altitude()) +", Speed: "+
			to_string(GPS::get_instance().get_velocity().speed) +", Course: "+
			to_string(GPS::get_instance().get_velocity().course) +", Sat: "+
			to_string(GPS::get_instance().get_satellites()) +", HDOP: "+
			to_string(GPS::get_instance().get_HDOP()) +", VDOP: "+
			to_string(GPS::get_instance().get_VDOP()));

		this_thread::sleep_for(50ms);
	}
}

State os::set_state(State new_state)
{
	ofstream state_file(STATE_FILE);
	state_file << new_state;
	state_file.close();

	return new_state;
}

string os::state_to_string(State state)
{
	switch (state)
	{
		case INITIALIZING:
			return "INITIALIZING";
		break;
		case ACQUIRING_FIX:
			return "ACQUIRING_FIX";
		break;
		case WAITING_LAUNCH:
			return "WAITING_LAUNCH";
		break;
		case GOING_UP:
			return "GOING_UP";
		break;
		case GOING_DOWN:
			return "GOING_DOWN";
		break;
		case LANDED:
			return "LANDED";
		break;
		case SHUT_DOWN:
			return "SHUT_DOWN";
	}
}
