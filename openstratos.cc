#include "openstratos.h"

int main(void)
{
	cout << "[OpenStratos] Starting..." << endl; // Only if verbose

	State last_state;
	if (file_exists(STATE_FILE))
		last_state = get_state();
	else
		last_state = SHUT_DOWN;

	State state = set_state(INITIALIZING);

	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm * now = gmtime(&timer.tv_sec);

	cout << "[OpenStratos] Current time: " << setfill('0') << setw(2) << now->tm_hour << ":" <<
		setfill('0') << setw(2) << now->tm_min << ":" << setfill('0') << setw(2) << now->tm_sec <<
		" UTC of " << setfill('0') << setw(2) << now->tm_mon << "/" <<
		setfill('0') << setw(2) << now->tm_mday << "/" << (now->tm_year+1900) << endl; // Only if verbose

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
			mkdir("data/logs/camera", 0755) == 0 && mkdir("data/logs/GPS", 0755) == 0
			&& mkdir("data/logs/GSM", 0755) == 0)
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
			exit(1);
		}
	}

	if (file_exists("data/img"))
	{
		logger.log("Image directory exists.");
	}
	else
	{
		logger.log("No image directory, creating...");
		if (mkdir("data/img", 0755) == 0)
		{
			logger.log("Image directory created.");
		}
		else
		{
			logger.log("Error creating image directory.");
			exit(1);
		}
	}

	logger.log("Available disk space: " + to_string(get_available_disk_space()/1073741824) + " GiB");
	if (get_available_disk_space() < FLIGHT_LENGTH*9437184000) // 1.25 times the flight length
	{
		logger.log("Error: Not enough disk space.");
		exit(1);
	}

	logger.log("Disk space enough for about " + to_string(get_available_disk_space()/7549747200) +
		" hours of fullHD video.");

	logger.log("Initializing WiringPi...");
	wiringPiSetup();
	logger.log("WiringPi initialized.");

	logger.log("Initializing GPS...");
	if ( ! GPS::get_instance().initialize())
	{
		logger.log("GPS initialization error.");
		exit(1);
	}
	logger.log("GPS initialized.");

	logger.log("Turning on GPS...");
	GPS::get_instance().turn_on();
	logger.log("GPS on.");

	logger.log("Initializing GSM...");
	if ( ! GSM::get_instance().initialize())
	{
		logger.log("GSM initialization error.");
		logger.log("Turning GPS off...");
		if (GPS::get_instance().turn_off())
			logger.log("GPS off.");
		else
			logger.log("Error turning GPS off.");

		exit(1);
	}
	logger.log("GSM initialized");

	logger.log("Checking batteries...");
	double main_battery, gsm_battery;
	GSM::get_instance().get_battery_status(main_battery, gsm_battery);
	logger.log("Batteries checked => Main battery: "+ to_string(main_battery) +
		"% - GSM battery: "+ to_string(gsm_battery) +"%");

	if ((main_battery < 0.95  && main_battery > -1) || gsm_battery < 0.95)
	{
		logger.log("Error: Not enough battery.");

		logger.log("Turning GSM off...");
		if (GSM::get_instance().turn_off())
			logger.log("GSM off.");
		else
			logger.log("Error turning GSM off.");

		logger.log("Turning GPS off...");
		if (GPS::get_instance().turn_off())
			logger.log("GPS off.");
		else
			logger.log("Error turning GPS off.");

		exit(1);
	}

	logger.log("Waiting for GSM connectivity...");
	this_thread::sleep_for(25ms);
	while ( ! GSM::get_instance().has_connectivity())
	{
		this_thread::sleep_for(1s);
	}
	this_thread::sleep_for(25ms);
	logger.log("GSM connected.");

	logger.log("Starting battery thread...");
	thread battery_thread(&battery_thread_fn, ref(state));
	logger.log("Battery thread started.");

	logger.log("Testing camera recording...");
	#ifndef RASPICAM
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
		logger.log("Turning GSM off...");
		if (GSM::get_instance().turn_off())
			logger.log("GSM off.");
		else
			logger.log("Error turning GSM off.");

		logger.log("Turning GPS off...");
		if (GPS::get_instance().turn_off())
			logger.log("GPS off.");
		else
			logger.log("Error turning GPS off.");

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
		logger.log("Turning GSM off...");
		if (GSM::get_instance().turn_off())
			logger.log("GSM off.");
		else
			logger.log("Error turning GSM off.");

		logger.log("Turning GPS off...");
		if (GPS::get_instance().turn_off())
			logger.log("GPS off.");
		else
			logger.log("Error turning GPS off.");

		exit(1);
	}
	logger.log("Recording started.");

	logger.log("Starting picture thread...");
	thread picture_thread(&picture_thread_fn, ref(state));
	logger.log("Picture thread started.");

	logger.log("Starting GPS Thread...");
	thread gps_thread(&gps_thread_fn, ref(state));
	logger.log("GPS thread started.");

	logger.log("Sending initialization SMS...");
	if ( ! GSM::get_instance().send_SMS("Initialization finished OK. Recording. Waiting for launch.", SMS_PHONE))
	{
		logger.log("Error sending initialization SMS.");

		logger.log("Stoping video recording.");
		if (Camera::get_instance().stop())
		{
			logger.log("Recording stopped.");
		}
		else
		{
			logger.log("Error stopping recording.");
		}

		logger.log("Turning GSM off...");
		if (GSM::get_instance().turn_off())
			logger.log("GSM off.");
		else
			logger.log("Error turning GSM off.");

		logger.log("Turning GPS off...");
		if (GPS::get_instance().turn_off())
			logger.log("GPS off.");
		else
			logger.log("Error turning GPS off.");

		exit(1);
	}
	logger.log("Initialization SMS sent.");

	state = set_state(WAITING_LAUNCH);
	logger.log("State changed to "+ state_to_string(state) +".");
	logger.log("Waiting for launch...");

	// Main logic
	int count = 0;
	while (state != SHUT_DOWN)
	{
		switch (state)
		{
			case WAITING_LAUNCH:
				while ( ! has_launched())
				{
					this_thread::sleep_for(1s);
				}
				this_thread::sleep_for(1min); // TODO delete
				logger.log("Balloon launched.");

				state = set_state(GOING_UP);
				logger.log("State changed to "+ state_to_string(state) +".");
			break;
			case GOING_UP:
				logger.log("Trying to send launch confirmation SMS...");
				if ( ! GSM::get_instance().send_SMS("Launched in Lat: "+
				to_string(GPS::get_instance().get_latitude()) +" and Lon: "+
				to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE))
				{
					logger.log("Error sending launch confirmation SMS.");
				}
				else
				{
					logger.log("Launch confirmation SMS sent.");
				}

				// while (GPS::get_instance().get_altitude() < 1500)
				// {
					this_thread::sleep_for(5s);
				// }
				this_thread::sleep_for(3min); // TODO: delete
				logger.log("1.5 km mark.");
				logger.log("Trying to send \"going up\" SMS...");
				if ( ! GSM::get_instance().send_SMS("1.5 km mark passed going up in Lat: "+
				to_string(GPS::get_instance().get_latitude()) +" and Lon: "+
				to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE))
				{
					logger.log("Error sending \"going up\" SMS.");
				}
				else
				{
					logger.log("\"Going up\" SMS sent.");
				}

				logger.log("Turning off GSM...");
				GSM::get_instance().turn_off();
				logger.log("GSM off.");

				this_thread::sleep_for(15min); // TODO delete

				while ( ! has_bursted());
				logger.log("Balloon burst.");
				state = set_state(GOING_DOWN);
				logger.log("State changed to "+ state_to_string(state) +".");
			break;
			case GOING_DOWN:
				while (GPS::get_instance().get_altitude() > 2500)
				{
					this_thread::sleep_for(5s);
				}
				this_thread::sleep_for(5min); // TODO: delete
				logger.log("2.5 km mark.");

				logger.log("Turning on GSM...");
				GSM::get_instance().turn_on();
				count = 0;
				while ( ! GSM::get_instance().is_up())
				{
					if (count > 5) break;
					this_thread::sleep_for(1s);
					++count;
				};

				if (GSM::get_instance().is_up())
				{
					logger.log("GSM on.");

					this_thread::sleep_for(3s); // Sleeping for letting GSM initialize
				}
				else
				{
					logger.log("GSM has not turned on yet, not waiting anymore.");
				}

				logger.log("Waiting for GSM connectivity...");
				count = 0;
				while ( ! GSM::get_instance().has_connectivity())
				{
					if (count > 15) break;
					this_thread::sleep_for(1s);
					++count;
				}
				if ( ! GSM::get_instance().has_connectivity())
				{
					logger.log("No connectivity, waiting for 1.5 km mark.");
				}
				else
				{
					logger.log("GSM connected.");

					logger.log("Trying to send first SMS...");
					if ( ! GSM::get_instance().send_SMS("2.5 km mark passed going down in Lat: "+ to_string(GPS::get_instance().get_latitude())
						+" and Lon: "+ to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE))
					{
						logger.log("Error sending first SMS.");
					}
					else
					{
						logger.log("First SMS sent.");
					}
				}

				while (GPS::get_instance().get_altitude() > 1500)// && ! has_landed())
				{
					this_thread::sleep_for(5s);
				}
				// if ( ! has_landed())
				// {
					logger.log("1.5 km mark.");

					if ( ! GSM::get_instance().is_up())
					{
						logger.log("GSM is off. Turning on GSM...");
						GSM::get_instance().turn_on();
						count = 0;
						while ( ! GSM::get_instance().is_up())
						{
							if (count > 5) break;
							this_thread::sleep_for(1s);
							++count;
						};

						if (GSM::get_instance().is_up())
						{
							logger.log("GSM on.");

							this_thread::sleep_for(3s); // Sleeping for letting GSM initialize
						}
						else
						{
							logger.log("GSM has not turned on yet, not waiting anymore.");
						}
					}

					count = 0;
					while ( ! GSM::get_instance().has_connectivity())
					{
						if (count > 15) break;
						this_thread::sleep_for(1s);
						++count;
					}
					if ( ! GSM::get_instance().has_connectivity())
					{
						logger.log("No connectivity, waiting for 500 m mark or landing.");
					}
					else
					{
						logger.log("GSM connected.");

						logger.log("Trying to send second SMS...");
						if ( ! GSM::get_instance().send_SMS("1.5 km mark passed going down in Lat: "+ to_string(GPS::get_instance().get_latitude())
							+" and Lon: "+ to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE))
						{
							logger.log("Error sending second SMS.");
						}
						else
						{
							logger.log("Second SMS sent.");
						}
					}
				// }

				while ( ! has_landed() && GPS::get_instance().get_altitude() > 500)
				{
					this_thread::sleep_for(5s);
				}

				if ( ! has_landed())
				{
					logger.log("500 m mark.");

					if ( ! GSM::get_instance().is_up())
					{
						logger.log("GSM is off. Turning on GSM...");
						GSM::get_instance().turn_on();
						count = 0;
						while ( ! GSM::get_instance().is_up())
						{
							if (count > 5) break;
							this_thread::sleep_for(1s);
							++count;
						};

						if (GSM::get_instance().is_up())
						{
							logger.log("GSM on.");

							this_thread::sleep_for(3s); // Sleeping for letting GSM initialize
						}
						else
						{
							logger.log("GSM has not turned on yet, not waiting anymore.");
						}
					}

					count = 0;
					while ( ! GSM::get_instance().has_connectivity())
					{
						if (count > 15) break;
						this_thread::sleep_for(1s);
						++count;
					}
					if ( ! GSM::get_instance().has_connectivity())
					{
						logger.log("No connectivity, waiting for landing.");
					}
					else
					{
						logger.log("GSM connected.");

						logger.log("Trying to send third SMS...");
						if ( ! GSM::get_instance().send_SMS("500 m mark passed in Lat: "+ to_string(GPS::get_instance().get_latitude())
							+" and Lon: "+ to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE))
						{
							logger.log("Error sending third SMS.");
						}
						else
						{
							logger.log("Third SMS sent.");
						}
					}
				}

				while ( ! has_landed())
				{
					this_thread::sleep_for(5s);
				}
				logger.log("Landed.");
				state = set_state(LANDED);
				logger.log("State changed to "+ state_to_string(state) +".");
			break;
			case LANDED:
				logger.log("Stopping video...");
				if ( ! Camera::get_instance().stop())
				{
					logger.log("Error stopping video.");
					// TODO try again?
				}
				else
				{
					logger.log("Video stopped.");
				}

				logger.log("Waiting 1 minute before sending landed SMS...");
				this_thread::sleep_for(1min);

				logger.log("Sending landed SMS...");
				if ( ! GSM::get_instance().send_SMS("Landed in Lat: "+ to_string(GPS::get_instance().get_latitude())
					+" and Lon: "+ to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE))
				{
					logger.log("Error sending landed SMS. Trying again in 10 minutes...");
				}
				else
				{
					logger.log("Landed SMS sent. Sending backup SMS in 10 minutes...");
				}

				this_thread::sleep_for(10min);

				logger.log("Sending second landed SMS...");

				while ( ! GSM::get_instance().send_SMS("Landed in Lat: "+ to_string(GPS::get_instance().get_latitude())
					+" and Lon: "+ to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE) &&
					(main_battery >= 0.05 || main_battery < -1) && gsm_battery >= 0.05)
				{
					logger.log("Error sending second SMS, trying again in 5 minutes.");
					this_thread::sleep_for(5min);
					GSM::get_instance().get_battery_status(main_battery, gsm_battery);
				}

				if ((main_battery < 0.05 && main_battery > -1) || gsm_battery < 0.05)
				{
					logger.log("Not enough battery.");
					logger.log("Main battery: "+ to_string(main_battery) +
						"% - GSM battery: "+ to_string(gsm_battery) +"%");
				}
				else
				{
					logger.log("Second SMS sent.");
				}

				logger.log("Shutting down...");
				state = set_state(SHUT_DOWN);
				logger.log("State changed to "+ state_to_string(state) +".");
		}
	}

	logger.log("Turning GSM off...");
	if (GSM::get_instance().turn_off())
		logger.log("GSM off.");
	else
		logger.log("Error turning GSM off.");

	logger.log("Turning GPS off...");
	if (GPS::get_instance().turn_off())
		logger.log("GPS off.");
	else
		logger.log("Error turning GPS off.");

	logger.log("Joining threads...");
	picture_thread.join();
	gps_thread.join();
	battery_thread.join();
	logger.log("Threads joined.");
	logger.log("Powering off...");
	sync();
	// reboot(RB_POWER_OFF);
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
			to_string(GPS::get_instance().get_satellites()) +", PDOP: "+
			to_string(GPS::get_instance().get_PDOP()));

		this_thread::sleep_for(250ms);
	}
}

void os::picture_thread_fn(State& state)
{
	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm * now = gmtime(&timer.tv_sec);

	Logger logger("data/logs/camera/Pictures."+ to_string(now->tm_year+1900) +"-"+ to_string(now->tm_mon) +"-"+
		to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+ to_string(now->tm_min) +"-"+
		to_string(now->tm_sec) +".log", "Pictures");

	logger.log("Waiting for launch...");

	while (state != GOING_UP)
	{
		this_thread::sleep_for(10s);
	}

	logger.log("Launched, waiting 2 minutes for first picture...");
	this_thread::sleep_for(2min);

	while (state == GOING_UP)
	{
		logger.log("Taking picture...");

		if ( ! Camera::get_instance().take_picture(generate_exif_data()))
		{
			logger.log("Error taking picture. Trying again in 30 seconds...");
		}
		else
		{
			logger.log("Picture taken correctly. Next picture in 30 seconds...");
		}

		this_thread::sleep_for(30s);
		logger.log("Taking picture...");

		if ( ! Camera::get_instance().take_picture(generate_exif_data()))
		{
			logger.log("Error taking picture. Next picture in 4 minutes...");
		}
		else
		{
			logger.log("Picture taken correctly. Next picture in 4 minutes...");
		}

		this_thread::sleep_for(4min);
	}

	logger.log("Going down, no more pictures are being taken, picture thread is closing.");
}

void os::battery_thread_fn(State& state)
{
	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm * now = gmtime(&timer.tv_sec);

	Logger logger("data/logs/GSM/Battery."+ to_string(now->tm_year+1900) +"-"+ to_string(now->tm_mon) +"-"+
		to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+ to_string(now->tm_min) +"-"+
		to_string(now->tm_sec) +".log", "Battery");

	double main_battery, gsm_battery;

	while (state != SHUT_DOWN)
	{
		if (GSM::get_instance().get_status())
		{
			GSM::get_instance().get_battery_status(main_battery, gsm_battery);
			logger.log("Main: "+ to_string(main_battery));
			logger.log("GSM: "+ to_string(gsm_battery));
		}

		this_thread::sleep_for(3min);
	}
}

State os::set_state(State new_state)
{
	ofstream state_file(STATE_FILE);
	state_file << state_to_string(new_state);
	state_file.close();

	return new_state;
}

State os::get_state()
{
	ifstream state_file(STATE_FILE);
	string str_state((istreambuf_iterator<char>(state_file)),
                 istreambuf_iterator<char>());
	state_file.close();

	if (str_state == "INITIALIZING") return INITIALIZING;
	if (str_state == "ACQUIRING_FIX") return ACQUIRING_FIX;
	if (str_state == "WAITING_LAUNCH") return WAITING_LAUNCH;
	if (str_state == "GOING_UP") return GOING_UP;
	if (str_state == "GOING_DOWN") return GOING_DOWN;
	if (str_state == "LANDED") return LANDED;
	if (str_state == "SHUT_DOWN") return SHUT_DOWN;
	if (str_state == "SAFE_MODE") return SAFE_MODE;

	return RECOVERY;
}

const string os::state_to_string(State state)
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
		break;
		case SAFE_MODE:
			return "SAFE_MODE";
		break;
		case RECOVERY:
			return "RECOVERY";
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
