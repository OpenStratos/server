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
	if (get_available_disk_space() < 22649241600) // Enough for about 3 hours of video
	{
		logger.log("Error: Not enough disk space.");
		cout << "[OpenStratos] Error: Not enough disk space." << endl;
		exit(1);
	}

	logger.log("Disk space enough for about " + to_string(get_available_disk_space()/7549747200) +
		" hours of fullHD video.");

	logger.log("Initializing WiringPi...");
	wiringPiSetup();
	logger.log("WiringPi initialized.");

	logger.log("Initializing GPS...");
	if ( ! GPS::get_instance().initialize(GPS_UART))
	{
		logger.log("GPS initialization error.");
		exit(1);
	}
	logger.log("GPS initialized.");

	// TODO start GSM and send message
	logger.log("Initializing GSM...");
	// if ( ! GSM::get_instance().initialize(GSM_PWR_GPIO, GSM_STATUS_GPIO, GSM_UART))
	// {
	// 	logger.log("GSM initialization error.");
	// 	exit(1);
	// }
	// GSM::get_instance().turn_on();
	logger.log("GSM initialized.");

	logger.log("Testing camera recording...");
	#ifndef RASPIVID
		logger.log("Error: No raspivid found. Is this a Raspberry?");
		exit(1);
	#endif
	logger.log("Recording 10 seconds as test...");
	if ( ! Camera::get_instance().record(10000))
	{
		logger.log("Error starting recording");
		exit(1);
	}
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

	logger.log("Starting video recording...");
	if ( ! Camera::get_instance().record())
	{
		logger.log("Error starting recording");
		exit(1);
	}
	logger.log("Recording started.");

	logger.log("Starting GPS Thread...");
	thread gps_thread(&gps_thread_fn, ref(state));
	logger.log("GPS thread started.");

	logger.log("Sending initialization SMS...");
	// if ( ! GSM::get_instance().send_SMS("Initialization finished OK. Recording. Waiting for launch.", SMS_PHONE))
	// {
	// 	logger.log("Error sending initialization SMS.");
	// 	exit(1);
	// }
	logger.log("Initialization SMS sent.");

	state = set_state(WAITING_LAUNCH);
	logger.log("State changed to "+ state_to_string(state) +".");
	logger.log("Waiting for launch...");

	// Main logic
	while (state != SHUT_DOWN)
	{
		switch (state)
		{
			case WAITING_LAUNCH:
				while ( ! has_launched());
				logger.log("Balloon launched.");

				logger.log("Trying to send launch confirmation SMS...");
				// if ( ! GSM::get_instance().send_SMS("Launched in Lat: "+
				// to_string(GPS::get_instance().get_latitude()) +" and Lon: "+
				// to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE))
				// {
				// 	logger.log("Error sending launch confirmation SMS.");
				// }
				// else
				// {
					logger.log("Launch confirmation SMS sent.");
				// }

				state = set_state(GOING_UP);
				logger.log("State changed to "+ state_to_string(state) +".");
			break;
			case GOING_UP:
				while (GPS::get_instance().get_altitude() < 1500)
				{
					this_thread::sleep_for(2s);
				}
				logger.log("1.5 km mark.");
				logger.log("Trying to send \"going up\" SMS...");
				// if ( ! GSM::get_instance().send_SMS("1.5 km mark passed going up in Lat: "+
				// to_string(GPS::get_instance().get_latitude()) +" and Lon: "+
				// to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE))
				// {
				// 	logger.log("Error sending \"going up\" SMS.");
				// }
				// else
				// {
					logger.log("\"Going up\" SMS sent.");
				// }

				logger.log("Turning off GSM...");
				// GSM::get_instance().turn_off();
				logger.log("GSM off.");

				while ( ! has_bursted());
				logger.log("Balloon burst.");
				state = set_state(GOING_DOWN);
				logger.log("State changed to "+ state_to_string(state) +".");
			break;
			case GOING_DOWN:
				while (GPS::get_instance().get_altitude() > 2500)
				{
					this_thread::sleep_for(10s);
				}
				logger.log("2.5 km mark.");
				logger.log("Turning on GSM...");
				// GSM::get_instance().turn_on();
				logger.log("GSM on.");
				logger.log("Trying to send first SMS...");
				// if ( ! GSM::get_instance().send_SMS("2.5 km mark passed in Lat: "+ to_string(GPS::get_instance().get_latitude())
				// 	+" and Lon: "+ to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE))
				// {
				// 	logger.log("Error sending first SMS.");
				// }
				// else
				// {
					logger.log("First SMS sent.");
				// }

				while (GPS::get_instance().get_altitude() > 1500)
				{
					this_thread::sleep_for(5s);
				}
				logger.log("1.5 km mark.");
				logger.log("Trying to send second SMS...");
				// if ( ! GSM::get_instance().send_SMS("1.5 km mark passed in Lat: "+ to_string(GPS::get_instance().get_latitude())
				// 	+" and Lon: "+ to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE))
				// {
				// 	logger.log("Error sending second SMS.");
				// }
				// else
				// {
					logger.log("Second SMS sent.");
				// }

				while ( ! has_landed() && GPS::get_instance().get_altitude() > 500);
				if ( ! has_landed())
				{
					logger.log("500 m mark.");
					logger.log("Trying to send third SMS...");
					// if ( ! GSM::get_instance().send_SMS("500 m mark passed in Lat: "+ to_string(GPS::get_instance().get_latitude())
					// 	+" and Lon: "+ to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE))
					// {
					// 	logger.log("Error sending third SMS.");
					// }
					// else
					// {
						logger.log("Third SMS sent.");
					// }
				}

				while ( ! has_landed());
				logger.log("Landed.");
				state = set_state(LANDED);
				logger.log("State changed to "+ state_to_string(state) +".");
			break;
			case LANDED:
				logger.log("Stopping video...");
				Camera::get_instance().stop();

				logger.log("Waiting 1 minute before sending landed SMS...");
				this_thread::sleep_for(1min);

				logger.log("Sending landed SMS...");
				// if ( ! GSM::get_instance().send_SMS("Landed in Lat: "+ to_string(GPS::get_instance().get_latitude())
				// 	+" and Lon: "+ to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE))
				// {
				// 	logger.log("Error sending landed SMS. Trying again in 15 minutes...");
				// }
				// else
				// {
					logger.log("Landed SMS sent. Sending backup SMS in 15 minutes...");
				// }

				this_thread::sleep_for(15min);

				logger.log("Sending second landed SMS...");

				// while ( ! GSM::get_instance().send_SMS("Landed in Lat: "+ to_string(GPS::get_instance().get_latitude())
				// 	+" and Lon: "+ to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE))
				// {
				// 	logger.log("Error sending second SMS, trying again in 5 minutes.");
				// 	this_thread::sleep_for(5min);
				// }

				logger.log("Second SMS sent.");

				logger.log("Shutting down...");
				state = set_state(SHUT_DOWN);
				logger.log("State changed to "+ state_to_string(state) +".");
		}
	}

	logger.log("Turning GSM off...");
	// GSM::get_instance().turn_off();
	logger.log("GSM off.");

	logger.log("Joining threads...");
	gps_thread.join();
	logger.log("Threads joined.");

	logger.log("Powering off...");
	sync();
	//reboot(RB_POWER_OFF);
	return 0;
}

inline bool os::file_exists(const string& name)
{
	struct stat buffer;
	return stat(name.c_str(), &buffer) == 0;
}

inline float os::get_available_disk_space()
{
	struct statvfs fs;
	statvfs("data", &fs);

	return ((float) fs.f_bsize)*fs.f_bavail;
}

void os::gps_thread_fn(State& state)
{
	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm * now = gmtime(&timer.tv_sec);

	Logger logger("data/logs/GPS/Position."+ to_string(now->tm_year+1900) +"-"+ to_string(now->tm_mon) +"-"+
		to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+ to_string(now->tm_min) +"-"+
		to_string(now->tm_sec) +".log", "GPSPosition");

	while (state != SHUT_DOWN) {
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

bool os::has_launched()
{
	double first_altitude = GPS::get_instance().get_altitude();
	this_thread::sleep_for(3s);
	double second_altitude = GPS::get_instance().get_altitude();

	return true;
	// return second_altitude > first_altitude + 10;
}

bool os::has_bursted()
{
	double first_altitude = GPS::get_instance().get_altitude();
	this_thread::sleep_for(3s);
	double second_altitude = GPS::get_instance().get_altitude();

	return true;
	// return second_altitude < first_altitude - 15;
}

bool os::has_landed()
{
	double first_altitude = GPS::get_instance().get_altitude();
	this_thread::sleep_for(5s);
	double second_altitude = GPS::get_instance().get_altitude();

	return true;
	// return first_altitude-second_altitude < 5;
}
