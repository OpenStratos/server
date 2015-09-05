#include "openstratos.h"

int main(void)
{
	#if DEBUG
		#ifdef SIM
			cout << "[OpenStratos] Simulation." << endl;
		#endif

		#ifdef REAL_SIM
			cout << "[OpenStratos] Realistic simulation." << endl;
		#endif

		cout << "[OpenStratos] Starting..." << endl;
	#endif

	if ( ! file_exists(STATE_FILE))
	{
		#if DEBUG
			cout << "[OpenStratos] No state file. Starting main logic..." << endl;
		#endif
		main_logic();
	}
	else
	{
		#if DEBUG
			cout << "[OpenStratos] State file found. Starting safe mode..." << endl;
		#endif
		safe_mode();
	}

	return 0;
}

void os::main_logic()
{
	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm* now = gmtime(&timer.tv_sec);

	#if DEBUG
		cout << "[OpenStratos] Current time: " << setfill('0') << setw(2) << now->tm_hour << ":" <<
			setfill('0') << setw(2) << now->tm_min << ":" << setfill('0') << setw(2) << now->tm_sec <<
			" UTC of " << setfill('0') << setw(2) << now->tm_mon << "/" <<
			setfill('0') << setw(2) << now->tm_mday << "/" << (now->tm_year+1900) << endl;
	#endif

	check_or_create("data");
	State state = set_state(INITIALIZING);

	check_or_create("data/logs");
	check_or_create("data/logs/main");
	check_or_create("data/logs/system");
	check_or_create("data/logs/camera");
	check_or_create("data/logs/GPS");
	check_or_create("data/logs/GSM");

	#if DEBUG
		cout << "[OpenStratos] Starting logger..." << endl;
	#endif

	Logger logger("data/logs/main/OpenStratos."+ to_string(now->tm_year+1900) +"-"+
		to_string(now->tm_mon) +"-"+ to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+
		to_string(now->tm_min) +"-"+ to_string(now->tm_sec) +".log", "OpenStratos");

	#if DEBUG
		cout << "[OpenStratos] Logger started." << endl;
	#endif

	logger.log("Starting system thread...");
	thread system_thread(&system_thread_fn, ref(state));
	logger.log("System thread started.");

	initialize(&logger, now);

	logger.log("Starting battery thread...");
	thread battery_thread(&battery_thread_fn, ref(state));
	logger.log("Battery thread started.");

	logger.log("Starting picture thread...");
	thread picture_thread(&picture_thread_fn, ref(state));
	logger.log("Picture thread started.");

	state = set_state(ACQUIRING_FIX);
	logger.log("State changed to "+ state_to_string(state) +".");

	main_while(&logger, &state);

	logger.log("Joining threads...");
	picture_thread.join();
	battery_thread.join();
	system_thread.join();
	logger.log("Threads joined.");

	shut_down(&logger);
}

void os::safe_mode()
{
	State last_state = get_last_state();
	State state = set_state(SAFE_MODE);
	Logger* logger;
	int count = 0;

	if (last_state > INITIALIZING)
	{
		struct timeval timer;
		gettimeofday(&timer, NULL);
		struct tm* now = gmtime(&timer.tv_sec);

		logger = new Logger("data/logs/main/OpenStratos."+ to_string(now->tm_year+1900) +"-"+
			to_string(now->tm_mon) +"-"+ to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+
			to_string(now->tm_min) +"-"+ to_string(now->tm_sec) +".log", "OpenStratos");
	}

	switch (last_state)
	{
		case INITIALIZING:
		case ACQUIRING_FIX:
			remove("data");
			reboot(RB_AUTOBOOT);
		break;
		case FIX_ACQUIRED: // It could be that the SMS was sent but the state didn't change
		case WAITING_LAUNCH:
			logger->log("Initializing WiringPi...");
			wiringPiSetup();
			logger->log("WiringPi initialized.");

			logger->log("Initializing GPS...");

			while ( ! GPS::get_instance().initialize() && ++count < 5)
				logger->log("GPS initialization error.");

			if (count < 5)
			{
				logger->log("GPS initialized.");
				count = 0;
				while ( ! GPS::get_instance().is_active() && ++count < 10)
					this_thread::sleep_for(10s);
				this_thread::sleep_for(10s);
				if (count == 10)
				{
					logger->log("Not getting fix. Going to recovery mode.");
					reboot(RB_AUTOBOOT);
				}

				double start_alt = GPS::get_instance().get_altitude();
				this_thread::sleep_for(5s);
				double end_alt = GPS::get_instance().get_altitude();

				if (end_alt - start_alt < -10) state = set_state(GOING_DOWN);
				else if (end_alt - start_alt > 10) state = set_state(GOING_UP);
				else if (end_alt > 5000) state = set_state(GOING_DOWN);
				else state = set_state(LANDED);

				logger->log("Initializing GSM...");
				if ( ! GSM::get_instance().initialize())
				{
					logger->log("GSM initialization error. Going to recovery mode.");
					reboot(RB_AUTOBOOT);
				}
				logger->log("GSM initialized.");

				main_while(logger, &state);
				shut_down(logger);
			}
			else
			{
				logger->log("Error initializing GPS. Going to recovery mode.");
				reboot(RB_AUTOBOOT);
			}
		break;
		case GOING_UP:
		break;
		case GOING_DOWN:
		break;
		case LANDED:
		break;
		case SHUT_DOWN:
		break;
		case SAFE_MODE:
			logger->log("Recovery mode");
			// TODO recovery mode
	}

	if (logger) delete logger;
}

void os::main_while(Logger* logger, State* state)
{
	while (*state != SHUT_DOWN)
	{
		if (*state == ACQUIRING_FIX)
		{
			aquire_fix(logger);
			*state = set_state(FIX_ACQUIRED);
			logger->log("State changed to "+ state_to_string(*state) +".");
		}
		else if (*state == FIX_ACQUIRED)
		{
			start_recording(logger);
			send_init_sms(logger);
			*state = set_state(WAITING_LAUNCH);
			logger->log("State changed to "+ state_to_string(*state) +".");
		}
		else if (*state == WAITING_LAUNCH)
		{
			wait_launch(logger);
			*state = set_state(GOING_UP);
			logger->log("State changed to "+ state_to_string(*state) +".");
		}
		else if (*state == GOING_UP)
		{
			go_up(logger);
			logger->log("Balloon burst.");

			*state = set_state(GOING_DOWN);
			logger->log("State changed to "+ state_to_string(*state) +".");
		}
		else if (*state == GOING_DOWN)
		{
			go_down(logger);
			*state = set_state(LANDED);
			logger->log("State changed to "+ state_to_string(*state) +".");
		}
		else if (*state == LANDED)
		{
			land(logger);
			*state = set_state(SHUT_DOWN);
			logger->log("State changed to "+ state_to_string(*state) +".");
		}
		else
		{
			reboot(RB_AUTOBOOT);
		}
	}
}

inline bool os::has_launched(double launch_altitude)
{
	double first_altitude = GPS::get_instance().get_altitude();
	if (first_altitude > launch_altitude + 100) return true;

	this_thread::sleep_for(5s);
	double second_altitude = GPS::get_instance().get_altitude();

	#if defined SIM || defined REAL_SIM
		return true;
	#else
		return second_altitude > first_altitude + 10;
	#endif
}

inline bool os::has_bursted(double maximum_altitude)
{
	double first_altitude = GPS::get_instance().get_altitude();
	if (first_altitude < maximum_altitude - 1000) return true;

	this_thread::sleep_for(6s);
	double second_altitude = GPS::get_instance().get_altitude();

	#if defined SIM || defined REAL_SIM
		return true;
	#else
		return second_altitude < first_altitude - 10;
	#endif
}

inline bool os::has_landed()
{
	double first_altitude = GPS::get_instance().get_altitude();
	this_thread::sleep_for(5s);
	double second_altitude = GPS::get_instance().get_altitude();

	return abs(first_altitude-second_altitude) < 5;
}

void os::initialize(Logger* logger, tm* now)
{
	check_or_create("data/video", logger);
	check_or_create("data/img", logger);

	float available_disk_space = get_available_disk_space();

	logger->log("Available disk space: " + to_string(available_disk_space/1073741824) + " GiB");
	if (available_disk_space < FLIGHT_LENGTH*9437184000) // 1.25 times the flight length
	{
		logger->log("Error: Not enough disk space.");
		sync();
		reboot(RB_POWER_OFF);
	}

	logger->log("Disk space enough for about " + to_string(available_disk_space/7549747200) +
		" hours of fullHD video.");

	logger->log("Initializing WiringPi...");
	wiringPiSetup();
	logger->log("WiringPi initialized.");

	logger->log("Initializing GPS...");
	if ( ! GPS::get_instance().initialize())
	{
		logger->log("GPS initialization error.");
		sync();
		reboot(RB_POWER_OFF);
	}
	logger->log("GPS initialized.");

	logger->log("Initializing GSM...");
	if ( ! GSM::get_instance().initialize())
	{
		logger->log("GSM initialization error.");
		logger->log("Turning GPS off...");
		if (GPS::get_instance().turn_off())
			logger->log("GPS off.");
		else
			logger->log("Error turning GPS off.");

		sync();
		reboot(RB_POWER_OFF);
	}
	logger->log("GSM initialized.");

	logger->log("Checking batteries...");
	double main_battery, gsm_battery;
	GSM::get_instance().get_battery_status(main_battery, gsm_battery);
	logger->log("Batteries checked => Main battery: "+ (main_battery > -1 ? to_string(main_battery*100)+"%" : "disconnected") +
		" - GSM battery: "+ to_string(gsm_battery*100) +"%");

	if ((main_battery < 0.95  && main_battery > -1) || gsm_battery < 0.95)
	{
		logger->log("Error: Not enough battery.");

		logger->log("Turning GSM off...");
		if (GSM::get_instance().turn_off())
			logger->log("GSM off.");
		else
			logger->log("Error turning GSM off.");

		logger->log("Turning GPS off...");
		if (GPS::get_instance().turn_off())
			logger->log("GPS off.");
		else
			logger->log("Error turning GPS off.");

		sync();
		reboot(RB_POWER_OFF);
	}

	logger->log("Waiting for GSM connectivity...");
	while ( ! GSM::get_instance().has_connectivity())
	{
		this_thread::sleep_for(1s);
	}
	logger->log("GSM connected.");

	logger->log("Testing camera recording...");
	#ifndef RASPICAM
		logger->log("Error: No raspivid found. Is this a Raspberry?");
		exit(1);
	#endif
	logger->log("Recording 10 seconds as test...");
	if ( ! Camera::get_instance().record(10000))
	{
		logger->log("Error starting recording");
		sync();
		reboot(RB_POWER_OFF);
	}
	this_thread::sleep_for(11s);
	if (file_exists("data/video/test.h264"))
	{
		logger->log("Camera test OK.");

		logger->log("Removing test file...");
		if (remove("data/video/test.h264"))
			logger->log("Error removing test file.");
		else
			logger->log("Test file removed.");
	}
	else
	{
		logger->log("Test recording error.");
		logger->log("Turning GSM off...");
		if (GSM::get_instance().turn_off())
			logger->log("GSM off.");
		else
			logger->log("Error turning GSM off.");

		logger->log("Turning GPS off...");
		if (GPS::get_instance().turn_off())
			logger->log("GPS off.");
		else
			logger->log("Error turning GPS off.");

		sync();
		reboot(RB_POWER_OFF);
	}
}

void os::aquire_fix(Logger* logger)
{
	while ( ! GPS::get_instance().is_active())
		this_thread::sleep_for(1s);

	logger->log("GPS fix acquired, waiting for date change.");
	this_thread::sleep_for(2s);

	struct timezone tz = {0, 0};
	tm gps_time = GPS::get_instance().get_time();
	struct timeval tv = {timegm(&gps_time), 0};
	settimeofday(&tv, &tz);

	logger->log("System date change.");
}

void os::start_recording(Logger* logger)
{
	logger->log("Starting video recording...");
	if ( ! Camera::get_instance().record())
	{
		logger->log("Error starting recording");
		logger->log("Turning GSM off...");
		if (GSM::get_instance().turn_off())
			logger->log("GSM off.");
		else
			logger->log("Error turning GSM off.");

		logger->log("Turning GPS off...");
		if (GPS::get_instance().turn_off())
			logger->log("GPS off.");
		else
			logger->log("Error turning GPS off.");

		sync();
		reboot(RB_POWER_OFF);
	}
	logger->log("Recording started.");
}

void os::send_init_sms(Logger* logger)
{
	logger->log("Sending initialization SMS...");
	if ( ! GSM::get_instance().send_SMS("Initialization finished OK. Recording. Waiting for launch.", SMS_PHONE))
	{
		logger->log("Error sending initialization SMS.");

		logger->log("Stoping video recording.");
		if (Camera::get_instance().stop())
		{
			logger->log("Recording stopped.");
		}
		else
		{
			logger->log("Error stopping recording.");
		}

		logger->log("Turning GSM off...");
		if (GSM::get_instance().turn_off())
			logger->log("GSM off.");
		else
			logger->log("Error turning GSM off.");

		logger->log("Turning GPS off...");
		if (GPS::get_instance().turn_off())
			logger->log("GPS off.");
		else
			logger->log("Error turning GPS off.");

		sync();
		reboot(RB_POWER_OFF);
	}
	logger->log("Initialization SMS sent.");
}

void os::wait_launch(Logger* logger)
{
	logger->log("Waiting for launch...");
	double launch_altitude = GPS::get_instance().get_altitude();
	#ifdef SIM
		this_thread::sleep_for(2min);
	#endif
	#ifdef REAL_SIM
		this_thread::sleep_for(20min);
	#endif

	while ( ! has_launched(launch_altitude))
		this_thread::sleep_for(1s);

	logger->log("Balloon launched.");
}

void os::go_up(Logger* logger)
{
	logger->log("Trying to send launch confirmation SMS...");
	if ( ! GSM::get_instance().send_SMS("Launched in Lat: "+
		to_string(GPS::get_instance().get_latitude()) +" and Lon: "+
		to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE))
	{
		logger->log("Error sending launch confirmation SMS.");
	}
	else
	{
		logger->log("Launch confirmation SMS sent.");
	}

	double maximum_altitude = 0;
	double current_altitude = GPS::get_instance().get_altitude();

	#if !defined SIM && !defined REAL_SIM
		while (current_altitude = GPS::get_instance().get_altitude() < 1500)
		{
			if (current_altitude > maximum_altitude) maximum_altitude = current_altitude;
			this_thread::sleep_for(2s);
		}
	#else
		this_thread::sleep_for(225s);
	#endif
	logger->log("1.5 km mark.");
	logger->log("Trying to send \"going up\" SMS...");
	if ( ! GSM::get_instance().send_SMS("1.5 km mark passed going up in Lat: "+
	to_string(GPS::get_instance().get_latitude()) +" and Lon: "+
	to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE))
	{
		logger->log("Error sending \"going up\" SMS.");
	}
	else
	{
		logger->log("\"Going up\" SMS sent.");
	}

	logger->log("Turning off GSM...");
	GSM::get_instance().turn_off();
	logger->log("GSM off.");

	bool bursted = false;

	#if defined SIM && !defined REAL_SIM
		this_thread::sleep_for(2min);
		logger->log("5 km mark passed going up.");
	#elif defined REAL_SIM && !defined SIM
		this_thread::sleep_for(1225s);
		logger->log("5 km mark passed going up.");
	#else
		while ( ! (bursted = has_bursted(maximum_altitude)) &&
			(current_altitude = GPS::get_instance().get_altitude()) < 5000)
		{
			if (current_altitude > maximum_altitude) maximum_altitude = current_altitude;
		}
		if ( ! bursted) logger->log("5 km mark passed going up.");
		else return;
	#endif

	#if defined SIM && !defined REAL_SIM
		this_thread::sleep_for(2min);
		logger->log("10 km mark passed going up.");
	#elif defined REAL_SIM && !defined SIM
		this_thread::sleep_for(1750s);
		logger->log("10 km mark passed going up.");
	#else
		while ( ! (bursted = has_bursted(maximum_altitude)) &&
			(current_altitude = GPS::get_instance().get_altitude()) < 10000)
		{
			if (current_altitude > maximum_altitude) maximum_altitude = current_altitude;
		}
		if ( ! bursted) logger->log("10 km mark passed going up.");
		else return;
	#endif

	#if defined SIM && !defined REAL_SIM
		this_thread::sleep_for(2min);
		logger->log("15 km mark passed going up.");
	#elif defined REAL_SIM && !defined SIM
		this_thread::sleep_for(1725s);
		logger->log("15 km mark passed going up.");
	#else
		while ( ! (bursted = has_bursted(maximum_altitude)) &&
			(current_altitude = GPS::get_instance().get_altitude()) < 15000)
		{
			if (current_altitude > maximum_altitude) maximum_altitude = current_altitude;
		}
		if ( ! bursted) logger->log("15 km mark passed going up.");
		else return;
	#endif

	#if defined SIM && !defined REAL_SIM
		this_thread::sleep_for(2min);
		logger->log("20 km mark passed going up.");
	#elif defined REAL_SIM && !defined SIM
		this_thread::sleep_for(1750s);
		logger->log("20 km mark passed going up.");
	#else
		while ( ! (bursted = has_bursted(maximum_altitude)) &&
			(current_altitude = GPS::get_instance().get_altitude()) < 20000)
		{
			if (current_altitude > maximum_altitude) maximum_altitude = current_altitude;
		}
		if ( ! bursted) logger->log("20 km mark passed going up.");
		else return;
	#endif

	#if defined SIM && !defined REAL_SIM
		this_thread::sleep_for(2min);
		logger->log("25 km mark passed going up.");
	#elif defined REAL_SIM && !defined SIM
		this_thread::sleep_for(1750s);
		logger->log("25 km mark passed going up.");
	#else
		while ( ! (bursted = has_bursted(maximum_altitude)) &&
			(current_altitude = GPS::get_instance().get_altitude()) < 25000)
		{
			if (current_altitude > maximum_altitude) maximum_altitude = current_altitude;
		}
		if ( ! bursted) logger->log("25 km mark passed going up.");
		else return;
	#endif

	#if defined SIM && !defined REAL_SIM
		this_thread::sleep_for(2min);
		logger->log("30 km mark passed going up.");
	#elif defined REAL_SIM && !defined SIM
		this_thread::sleep_for(1725s);
		logger->log("30 km mark passed going up.");
	#else
		while ( ! (bursted = has_bursted(maximum_altitude)) &&
			(current_altitude = GPS::get_instance().get_altitude()) < 30000)
		{
			if (current_altitude > maximum_altitude) maximum_altitude = current_altitude;
		}
		if ( ! bursted) logger->log("30 km mark passed going up.");
		else return;
	#endif

	#if defined SIM && !defined REAL_SIM
		this_thread::sleep_for(2min);
	#elif defined REAL_SIM && !defined SIM
		this_thread::sleep_for(1650s);
	#else
		while ( ! (bursted = has_bursted(maximum_altitude)) &&
			(current_altitude = GPS::get_instance().get_altitude()) < 35000)
		{
			if (current_altitude > maximum_altitude) maximum_altitude = current_altitude;
		}
		if ( ! bursted) logger->log("35 km mark passed going up.");
		else return;
	#endif

	while ( ! has_bursted(maximum_altitude))
		if ((current_altitude = GPS::get_instance().get_altitude()) > maximum_altitude)
			maximum_altitude = current_altitude;
}

void os::go_down(Logger* logger)
{
	#if defined SIM && !defined REAL_SIM
		this_thread::sleep_for(1min);
		logger->log("25 km mark passed going down.");
	#elif defined REAL_SIM && !defined SIM
		this_thread::sleep_for(325s);
		logger->log("25 km mark passed going down.");
	#else
		while (GPS::get_instance().get_altitude() > 25000)
			this_thread::sleep_for(5s);

		logger->log("25 km mark passed going down.");
	#endif

	#if defined SIM && !defined REAL_SIM
		this_thread::sleep_for(1min);
		logger->log("15 km mark passed going down.");
	#elif defined REAL_SIM && !defined SIM
		this_thread::sleep_for(700s);
		logger->log("15 km mark passed going down.");
	#else
		while (GPS::get_instance().get_altitude() > 15000)
			this_thread::sleep_for(5s);

		logger->log("15 km mark passed going down.");
	#endif

	#if defined SIM && !defined REAL_SIM
		this_thread::sleep_for(2min);
		logger->log("5 km mark passed going down.");
	#elif defined REAL_SIM && !defined SIM
		this_thread::sleep_for(1450s);
		logger->log("5 km mark passed going down.");
	#else
		while (GPS::get_instance().get_altitude() > 5000)
			this_thread::sleep_for(5s);

		logger->log("5 km mark passed going down.");
	#endif

	#if defined SIM && !defined REAL_SIM
		this_thread::sleep_for(1min);
		logger->log("2.5 km mark passed going down.");
	#elif defined REAL_SIM && !defined SIM
		this_thread::sleep_for(525s);
		logger->log("2.5 km mark passed going down.");
	#else
		while (GPS::get_instance().get_altitude() > 2500)
			this_thread::sleep_for(5s);

		logger->log("2.5 km mark passed going down.");
	#endif

	logger->log("Turning on GSM...");
	GSM::get_instance().turn_on();

	logger->log("Waiting for GSM connectivity...");
	int count = 0;
	while ( ! GSM::get_instance().has_connectivity())
	{
		if (count == 20) break;
		this_thread::sleep_for(1s);
		++count;
	}
	if (count == 20)
	{
		logger->log("No connectivity, waiting for 1.5 km mark or landing.");
	}
	else
	{
		logger->log("GSM connected.");

		logger->log("Trying to send first SMS...");
		if ( ! GSM::get_instance().send_SMS("2.5 km mark passed going down in Lat: "+ to_string(GPS::get_instance().get_latitude())
			+" and Lon: "+ to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE))
		{
			logger->log("Error sending first SMS.");
		}
		else
		{
			logger->log("First SMS sent.");
		}
	}

	bool landed = false;

	#if defined SIM && !defined REAL_SIM
		this_thread::sleep_for(1min);
		logger->log("1.5 km mark passed going down.");
	#elif defined REAL_SIM && !defined SIM
		this_thread::sleep_for(225s);
		logger->log("1.5 km mark passed going down.");
	#else
		while (GPS::get_instance().get_altitude() > 1500 && ! (landed = has_landed()));
		if ( ! landed) logger->log("1.5 km mark passed going down.");
	#endif

	if ( ! landed)
	{
		count = 0;
		while ( ! GSM::get_instance().has_connectivity())
		{
			if (count == 20) break;
			this_thread::sleep_for(1s);
			++count;
		}
		if (count == 20)
		{
			logger->log("No connectivity, waiting for 500 m mark or landing.");
		}
		else
		{
			logger->log("GSM connected.");

			logger->log("Trying to send second SMS...");
			if ( ! GSM::get_instance().send_SMS("1.5 km mark passed going down in Lat: "+ to_string(GPS::get_instance().get_latitude())
				+" and Lon: "+ to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE))
			{
				logger->log("Error sending second SMS.");
			}
			else
			{
				logger->log("Second SMS sent.");
			}
		}
	}

	#if defined SIM && !defined REAL_SIM
		this_thread::sleep_for(1min);
		logger->log("500 m mark passed going down.");
	#elif defined REAL_SIM && !defined SIM
		this_thread::sleep_for(175s);
		logger->log("500 m mark passed going down.");
	#else
		while (GPS::get_instance().get_altitude() > 500 && ! (landed = has_landed()));
		if ( ! landed) logger->log("500 m mark passed going down.");
	#endif

	if ( ! landed)
	{
		logger->log("500 m mark.");

		count = 0;
		while ( ! GSM::get_instance().has_connectivity())
		{
			if (count > 15) break;
			this_thread::sleep_for(1s);
			++count;
		}
		if ( ! GSM::get_instance().has_connectivity())
		{
			logger->log("No connectivity, waiting for landing.");
		}
		else
		{
			logger->log("GSM connected.");

			logger->log("Trying to send third SMS...");
			if ( ! GSM::get_instance().send_SMS("500 m mark passed in Lat: "+ to_string(GPS::get_instance().get_latitude())
				+" and Lon: "+ to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE))
			{
				logger->log("Error sending third SMS.");
			}
			else
			{
				logger->log("Third SMS sent.");
			}
		}
	}

	while ( ! has_landed());
	logger->log("Landed.");
}

void os::land(Logger* logger)
{
	logger->log("Stopping video...");
	if ( ! Camera::get_instance().stop())
		logger->log("Error stopping video.");
	else
		logger->log("Video stopped.");

	logger->log("Waiting 1 minute before sending landed SMS...");
	this_thread::sleep_for(1min);

	logger->log("Sending landed SMS...");
	if ( ! GSM::get_instance().send_SMS("Landed in Lat: "+ to_string(GPS::get_instance().get_latitude())
		+" and Lon: "+ to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE))
	{
		logger->log("Error sending landed SMS. Trying again in 10 minutes...");
	}
	else
	{
		logger->log("Landed SMS sent. Sending backup SMS in 10 minutes...");
	}

	this_thread::sleep_for(10min);

	logger->log("Sending second landed SMS...");

	double main_battery = 1, gsm_battery = 1;
	while ( ! GSM::get_instance().send_SMS("Landed in Lat: "+ to_string(GPS::get_instance().get_latitude())
		+" and Lon: "+ to_string(GPS::get_instance().get_longitude()) +".", SMS_PHONE) &&
		(main_battery >= 0.05 || main_battery < -1) && gsm_battery >= 0.05)
	{
		logger->log("Error sending second SMS, trying again in 5 minutes.");
		this_thread::sleep_for(5min);
		GSM::get_instance().get_battery_status(main_battery, gsm_battery);
	}

	if ((main_battery < 0.05 && main_battery > -1) || gsm_battery < 0.05)
	{
		logger->log("Not enough battery.");
		logger->log("Main battery: "+ to_string(main_battery) +
			"% - GSM battery: "+ to_string(gsm_battery) +"%");
	}
	else
	{
		logger->log("Second SMS sent.");
	}
}

void os::shut_down(Logger* logger)
{
	logger->log("Shutting down...");

	logger->log("Turning GSM off...");
	if (GSM::get_instance().turn_off())
		logger->log("GSM off.");
	else
		logger->log("Error turning GSM off.");

	logger->log("Turning GPS off...");
	if (GPS::get_instance().turn_off())
		logger->log("GPS off.");
	else
		logger->log("Error turning GPS off.");

	logger->log("Powering off...");
	sync();
	reboot(RB_POWER_OFF);
}
