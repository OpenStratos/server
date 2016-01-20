#include "openstratos.h"

int main(void)
{
	#ifdef DEBUG
		#ifdef SIM
			cout << "[OpenStratos] Simulation." << endl;
		#endif

		#ifdef REAL_SIM
			cout << "[OpenStratos] Realistic simulation." << endl;
		#endif

		cout << "[OpenStratos] Starting " << PACKAGE_STRING << "..." << endl;
	#endif

	if ( ! file_exists(STATE_FILE))
	{
		#ifdef DEBUG
			cout << "[OpenStratos] No state file. Starting main logic..." << endl;
		#endif
		main_logic();
	}
	else
	{
		#ifdef DEBUG
			cout << "[OpenStratos] State file found. Starting safe mode..." << endl;
		#endif
		safe_mode();
	}

	#ifndef NO_POWER_OFF
		sync();
		reboot(RB_POWER_OFF);
	#endif

	return 0;
}

void os::main_logic()
{
	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm* now = gmtime(&timer.tv_sec);

	#ifdef DEBUG
		cout << "[OpenStratos] Current time: " << setfill('0') << setw(2) << now->tm_hour << ":" <<
			setfill('0') << setw(2) << now->tm_min << ":" << setfill('0') << setw(2) << now->tm_sec
			<< " UTC of " << setfill('0') << setw(2) << now->tm_mon << "/" <<
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

	#ifdef DEBUG
		cout << "[OpenStratos] Starting logger..." << endl;
	#endif

	Logger logger("data/logs/main/OpenStratos."+ to_string(now->tm_year+1900) +"-"+
		to_string(now->tm_mon) +"-"+ to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+
		to_string(now->tm_min) +"-"+ to_string(now->tm_sec) +".log", "OpenStratos");

	#ifdef DEBUG
		cout << "[OpenStratos] Logger started." << endl;
	#endif

	logger.log(PACKAGE_STRING);

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
	double latitude = 0, longitude = 0;

	check_or_create("data/logs");
	check_or_create("data/logs/main");
	check_or_create("data/logs/system");
	check_or_create("data/logs/camera");
	check_or_create("data/logs/GPS");
	check_or_create("data/logs/GSM");

	if (last_state > ACQUIRING_FIX)
	{
		struct timeval timer;
		gettimeofday(&timer, NULL);
		struct tm* now = gmtime(&timer.tv_sec);

		logger = new Logger("data/logs/main/OpenStratos."+ to_string(now->tm_year+1900) +"-"+
			to_string(now->tm_mon) +"-"+ to_string(now->tm_mday) +"."+ to_string(now->tm_hour)
			+"-"+ to_string(now->tm_min) +"-"+ to_string(now->tm_sec) +".log", "OpenStratos");

		logger->log(PACKAGE_STRING);
	}

	switch (last_state)
	{
		case INITIALIZING:
		case ACQUIRING_FIX:
			remove(STATE_FILE);
			#ifndef NO_POWER_OFF
				sync();
				reboot(RB_AUTOBOOT);
			#else
				exit(0);
			#endif
		break;
		case FIX_ACQUIRED: // It could be that the SMS was sent but the state didn't change
		case WAITING_LAUNCH:
		case GOING_UP:
		case GOING_DOWN:
		case LANDED:
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
				logger->log("Waiting for GPS fix...");
				while ( ! GPS::get_instance().is_fixed() && ++count < 100)
					this_thread::sleep_for(10s);

				if (count == 100)
				{
					logger->log("Not getting fix. Going to recovery mode.");
					delete logger;
					#ifndef NO_POWER_OFF
						sync();
						reboot(RB_AUTOBOOT);
					#else
						exit(1);
					#endif
				}

				logger->log("GPS fix acquired.");
				this_thread::sleep_for(1min);
				state = (state == LANDED) ? LANDED : get_real_state();

				logger->log("Initializing GSM...");
				if ( ! GSM::get_instance().initialize())
				{
					logger->log("GSM initialization error. Going to recovery mode.");
					delete logger;
					#ifndef NO_POWER_OFF
						sync();
						reboot(RB_AUTOBOOT);
					#else
						exit(1);
					#endif
				}
				logger->log("GSM initialized.");

				if (state != LANDED)
				{
					logger->log("Trying to start recording...");
					if (Camera::get_instance().record())
						logger->log("Recording.");
					else
						logger->log("Error starting recording");
				}

				main_while(logger, &state);
				shut_down(logger);
			}
			else
			{
				logger->log("Error initializing GPS. Going to recovery mode.");
				delete logger;
				#ifndef NO_POWER_OFF
					sync();
					reboot(RB_AUTOBOOT);
				#else
					exit(1);
				#endif
			}
		break;
		case SHUT_DOWN:
			shut_down(logger);
		break;
		case SAFE_MODE:
			logger->log("Recovery mode");

			logger->log("Initializing WiringPi...");
			wiringPiSetup();
			logger->log("WiringPi initialized.");

			logger->log("Initializing GSM...");
			while ( ! GSM::get_instance().initialize())
				logger->log("GSM initialization error.");
			logger->log("GSM initialized");
			logger->log("Waiting for GSM connectivity...");
			while ( ! GSM::get_instance().has_connectivity())
				this_thread::sleep_for(5s);
			logger->log("GSM connected.");

			logger->log("Sending mayday messages...");
			for (count = 0; count < 2;)
			{
				this_thread::sleep_for(5min);

				GSM::get_instance().get_location(latitude, longitude);
				GSM::get_instance().send_SMS("MAYDAY\r\nLat: "+ to_string(latitude) +"\r\n"+
					"Lon: "+ to_string(longitude), SMS_PHONE) && ++count;
			}
			logger->log("Mayday messages sent.");

			logger->log("Initializing GPS...");
			count = 0;
			while ( ! GPS::get_instance().initialize() && ++count < 5)
				logger->log("GPS initialization error.");

			if (count < 5)
			{
				logger->log("GPS initialized.");
				count = 0;
				logger->log("Waiting for GPS fix...");
				while ( ! GPS::get_instance().is_fixed() && ++count < 100)
					this_thread::sleep_for(10s);

				if (count == 100)
				{
					logger->log("Not getting fix.");
					shut_down(logger);
				}

				logger->log("GPS fix acquired.");
				this_thread::sleep_for(5s);
				for (int i = 0;
					GPS::get_instance().get_HDOP() > 5 && i < 10;
					i++)
				{
					this_thread::sleep_for(500ms);
				}

				while ( ! GSM::get_instance().send_SMS("MAYDAY\r\nLat: " +
					to_string(GPS::get_instance().get_latitude()) +
					"\r\nLon: "+ to_string(GPS::get_instance().get_longitude()) +
					"\r\nAlt: "+ to_string(GPS::get_instance().get_altitude()) +
					"\r\nFix: OK", SMS_PHONE))
				{
					this_thread::sleep_for(1min);
				}

				this_thread::sleep_for(30s);
				state = (state == LANDED) ? LANDED : get_real_state();

				main_while(logger, &state);
				shut_down(logger);
			}
			else
			{
				shut_down(logger);
			}
	}

	if (logger) delete logger;
}

void os::main_while(Logger* logger, State* state)
{
	double launch_altitude;

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
			this_thread::sleep_for(2min);
			logger->log("Sleeping 2 minutes for fix stabilization.");

			start_recording(logger);
			send_init_sms(logger);
			*state = set_state(WAITING_LAUNCH);
			logger->log("State changed to "+ state_to_string(*state) +".");
		}
		else if (*state == WAITING_LAUNCH)
		{
			wait_launch(logger, launch_altitude);
			*state = set_state(GOING_UP);
			logger->log("State changed to "+ state_to_string(*state) +".");
		}
		else if (*state == GOING_UP)
		{
			go_up(logger, launch_altitude);
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
			#ifndef NO_POWER_OFF
				sync();
				reboot(RB_AUTOBOOT);
			#else
				exit(1);
			#endif
		}
	}
}

void os::initialize(Logger* logger, tm* now)
{
	check_or_create("data/video", logger);
	check_or_create("data/img", logger);

	float available_disk_space = get_available_disk_space();

	logger->log("Available disk space: " + to_string(available_disk_space/1073741824) + " GiB");
	if (available_disk_space < (FLIGHT_LENGTH*1.25+0.5)*7549747200)
	{
		logger->log("Error: Not enough disk space.");

		#ifndef NO_POWER_OFF
			sync();
			reboot(RB_POWER_OFF);
		#else
			exit(1);
		#endif
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

		#ifndef NO_POWER_OFF
			sync();
			reboot(RB_POWER_OFF);
		#else
			exit(1);
		#endif
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

		#ifndef NO_POWER_OFF
			sync();
			reboot(RB_POWER_OFF);
		#else
			exit(1);
		#endif
	}
	logger->log("GSM initialized.");

	logger->log("Checking batteries...");
	double main_battery, gsm_battery;
	if ( ! GSM::get_instance().get_battery_status(main_battery, gsm_battery) &&
		 ! GSM::get_instance().get_battery_status(main_battery, gsm_battery))
	{
		logger->log("Error checking batteries.");

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

		#ifndef NO_POWER_OFF
			sync();
			reboot(RB_POWER_OFF);
		#else
			exit(1);
		#endif
	}

	logger->log("Batteries checked => Main battery: "+ (main_battery > -1 ? to_string(main_battery*100)+"%" : "disconnected") +
		" - GSM battery: "+ to_string(gsm_battery*100) +"%");

	if ((main_battery < 0.9  && main_battery > -1) || gsm_battery < 0.85)
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

		#ifndef NO_POWER_OFF
			sync();
			reboot(RB_POWER_OFF);
		#else
			exit(1);
		#endif
	}

	logger->log("Waiting for GSM connectivity...");
	while ( ! GSM::get_instance().has_connectivity())
		this_thread::sleep_for(1s);

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

		#ifndef NO_POWER_OFF
			sync();
			reboot(RB_POWER_OFF);
		#else
			exit(1);
		#endif
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

		#ifndef NO_POWER_OFF
			sync();
			reboot(RB_POWER_OFF);
		#else
			exit(1);
		#endif
	}
}

void os::aquire_fix(Logger* logger)
{
	while ( ! GPS::get_instance().is_fixed())
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

		#ifndef NO_POWER_OFF
			sync();
			reboot(RB_POWER_OFF);
		#else
			exit(1);
		#endif
	}
	logger->log("Recording started.");
}

void os::send_init_sms(Logger* logger)
{
	double main_battery = 0, gsm_battery = 0;
	bool bat_status = false;

	logger->log("Getting battery values...");
	if (bat_status = GSM::get_instance().get_battery_status(main_battery, gsm_battery))
		logger->log("Battery status received.");
	else
		logger->log("Error getting battery status.");

	logger->log("Sending initialization SMS...");
	while (GPS::get_instance().get_PDOP() > 5)
		this_thread::sleep_for(1s);

	if ( ! GSM::get_instance().send_SMS(
		"Init: OK\r\nAlt: "+ to_string((int) GPS::get_instance().get_altitude()) +
		" m\r\nLat: "+ to_string(GPS::get_instance().get_latitude()) +"\r\n"+
		"Lon: "+ to_string(GPS::get_instance().get_longitude()) +"\r\n"+
		(bat_status ? "Main bat: "+ to_string((int) (main_battery*100)) +"%\r\n"+
			"GSM bat: "+ to_string((int) (gsm_battery*100)) +"%\r\n" : "Bat: ERR\r\n") +
		"Fix: "+ (GPS::get_instance().is_fixed() ? "OK" : "ERR") +
		"\r\nSat: "+ to_string(GPS::get_instance().get_satellites()) +
		"\r\nWaiting Launch", SMS_PHONE))
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

		#ifndef NO_POWER_OFF
			sync();
			reboot(RB_POWER_OFF);
		#else
			exit(1);
		#endif
	}
	logger->log("Initialization SMS sent.");
}

void os::wait_launch(Logger* logger, double& launch_altitude)
{
	logger->log("Waiting for launch...");
	launch_altitude = GPS::get_instance().get_altitude();
	#ifdef SIM
		this_thread::sleep_for(2min);
	#endif
	#ifdef REAL_SIM
		this_thread::sleep_for(10min);
	#endif

	while ( ! has_launched(launch_altitude))
		this_thread::sleep_for(1s);

	logger->log("Balloon launched.");
}

void os::go_up(Logger* logger, double launch_altitude)
{
	double main_battery = 0, gsm_battery = 0;
	bool bat_status = false;

	logger->log("Getting battery values...");
	if (bat_status = GSM::get_instance().get_battery_status(main_battery, gsm_battery))
		logger->log("Battery status received.");
	else
		logger->log("Error getting battery status.");

	logger->log("Trying to send launch confirmation SMS...");
	for (int i = 0; GPS::get_instance().get_PDOP() > 5 && i < 5; i++)
		this_thread::sleep_for(500ms);

	if ( ! GSM::get_instance().send_SMS(
		"Launch\r\nAlt: "+ to_string((int) launch_altitude) +
		" m\r\nLat: "+ to_string(GPS::get_instance().get_latitude()) +"\r\n"+
		"Lon: "+ to_string(GPS::get_instance().get_longitude()) +"\r\n"+
		(bat_status ? "Main bat: "+ to_string((int) (main_battery*100)) +"%\r\n"+
			"GSM bat: "+ to_string((int) (gsm_battery*100)) +"%\r\n" : "Bat: ERR\r\n") +
		"Fix: "+ (GPS::get_instance().is_fixed() ? "OK" : "ERR") +
		"\r\nSat: "+ to_string(GPS::get_instance().get_satellites()), SMS_PHONE))
	{
		logger->log("Error sending launch confirmation SMS.");
	}
	else
	{
		logger->log("Launch confirmation SMS sent.");
	}

	for (int i = 0; GPS::get_instance().get_PDOP() > 5 && i < 5; i++)
		this_thread::sleep_for(500ms);
	double maximum_altitude = GPS::get_instance().get_altitude();

	wait_up_for(1200, maximum_altitude);
	logger->log("1.2 km mark.");

	logger->log("Getting battery values...");
	if (bat_status = GSM::get_instance().get_battery_status(main_battery, gsm_battery))
		logger->log("Battery status received.");
	else
		logger->log("Error getting battery status.");

	logger->log("Trying to send \"going up\" SMS...");
	for (int i = 0; GPS::get_instance().get_PDOP() > 5 && i < 5; i++)
		this_thread::sleep_for(500ms);

	if ( ! GSM::get_instance().send_SMS(
		"Alt: "+ to_string((int) GPS::get_instance().get_altitude()) +
		" m\r\nLat: "+ to_string(GPS::get_instance().get_latitude()) +"\r\n"+
		"Lon: "+ to_string(GPS::get_instance().get_longitude()) +"\r\n"+
		(bat_status ? "Main bat: "+ to_string((int) (main_battery*100)) +"%\r\n"+
			"GSM bat: "+ to_string((int) (gsm_battery*100)) +"%\r\n" : "Bat: ERR\r\n") +
		"Fix: "+ (GPS::get_instance().is_fixed() ? "OK" : "ERR") +
		"\r\nSat: "+ to_string(GPS::get_instance().get_satellites()), SMS_PHONE) &&
		// Second attempt
		! GSM::get_instance().send_SMS(
			"Alt: "+ to_string((int) GPS::get_instance().get_altitude()) +
			" m\r\nLat: "+ to_string(GPS::get_instance().get_latitude()) +"\r\n"+
			"Lon: "+ to_string(GPS::get_instance().get_longitude()) +"\r\n"+
			(bat_status ? "Main bat: "+ to_string((int) (main_battery*100)) +"%\r\n"+
				"GSM bat: "+ to_string((int) (gsm_battery*100)) +"%\r\n" : "Bat: ERR\r\n") +
			"Fix: "+ (GPS::get_instance().is_fixed() ? "OK" : "ERR") +
			"\r\nSat: "+ to_string(GPS::get_instance().get_satellites()), SMS_PHONE))
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

	bursted = wait_up_for(5000, maximum_altitude);
	#if defined SIM || defined REAL_SIM
		logger->log("5 km mark passed going up.");
	#else
		if ( ! bursted)
			logger->log("5 km mark passed going up.");
		else return;
	#endif

	bursted = wait_up_for(10000, maximum_altitude);
	#if defined SIM || defined REAL_SIM
		logger->log("10 km mark passed going up.");
	#else
		if ( ! bursted)
			logger->log("10 km mark passed going up.");
		else return;
	#endif

	bursted = wait_up_for(15000, maximum_altitude);
	#if defined SIM || defined REAL_SIM
		logger->log("15 km mark passed going up.");
	#else
		if ( ! bursted)
			logger->log("15 km mark passed going up.");
		else return;
	#endif

	bursted = wait_up_for(20000, maximum_altitude);
	#if defined SIM || defined REAL_SIM
		logger->log("20 km mark passed going up.");
	#else
		if ( ! bursted)
			logger->log("20 km mark passed going up.");
		else return;
	#endif

	bursted = wait_up_for(25000, maximum_altitude);
	#if defined SIM || defined REAL_SIM
		logger->log("25 km mark passed going up.");
	#else
		if ( ! bursted)
			logger->log("25 km mark passed going up.");
		else return;
	#endif

	bursted = wait_up_for(30000, maximum_altitude);
	#if defined SIM || defined REAL_SIM
		logger->log("30 km mark passed going up.");
	#else
		if ( ! bursted)
			logger->log("30 km mark passed going up.");
		else return;
	#endif

	bursted = wait_up_for(35000, maximum_altitude);
	#if defined SIM || defined REAL_SIM
		logger->log("35 km mark passed going up.");
	#else
		if ( ! bursted)
			logger->log("35 km mark passed going up.");
		else return;
	#endif

	while ( ! has_bursted(maximum_altitude))
	{
		double current_altitude;
		for (int i = 0; GPS::get_instance().get_VDOP() > 5 && i < 5; i++)
			this_thread::sleep_for(500ms);

		if ((current_altitude = GPS::get_instance().get_altitude()) > maximum_altitude)
			maximum_altitude = current_altitude;
	}

	logger->log("Balloon burst at about "+ to_string((int) maximum_altitude) +" m.");
}

void os::go_down(Logger* logger)
{
	double main_battery = 0, gsm_battery = 0;
	bool bat_status = false;

	wait_down_for(25000);
	logger->log("25 km mark passed going down.");

	wait_down_for(15000);
	logger->log("15 km mark passed going down.");

	wait_down_for(5000);
	logger->log("5 km mark passed going down.");

	wait_down_for(2000);
	logger->log("2 km mark passed going down.");

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
		logger->log("No connectivity, waiting for 1.2 km mark or landing.");
	}
	else
	{
		logger->log("GSM connected.");

		logger->log("Getting battery values...");
			if (bat_status = GSM::get_instance().get_battery_status(main_battery, gsm_battery))
				logger->log("Battery status received.");
			else
				logger->log("Error getting battery status.");

		logger->log("Trying to send first SMS...");
		for (int i = 0; GPS::get_instance().get_PDOP() > 5 && i < 5; i++)
			this_thread::sleep_for(500ms);

		if ( ! GSM::get_instance().send_SMS(
			"Alt: "+ to_string((int) GPS::get_instance().get_altitude()) +
			" m\r\nLat: "+ to_string(GPS::get_instance().get_latitude()) +"\r\n"+
			"Lon: "+ to_string(GPS::get_instance().get_longitude()) +"\r\n"+
			(bat_status ? "Main bat: "+ to_string((int) (main_battery*100)) +"%\r\n"+
				"GSM bat: "+ to_string((int) (gsm_battery*100)) +"%\r\n" : "Bat: ERR\r\n") +
			"Fix: "+ (GPS::get_instance().is_fixed() ? "OK" : "ERR") +
			"\r\nSat: "+ to_string(GPS::get_instance().get_satellites()), SMS_PHONE))
		{
			logger->log("Error sending first SMS.");
		}
		else
		{
			logger->log("First SMS sent.");
		}

		bat_status = false;
	}

	if ( ! wait_down_for(1200))
	{
		logger->log("1.2 km mark passed going down.");

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

			logger->log("Getting battery values...");
			if (bat_status = GSM::get_instance().get_battery_status(main_battery, gsm_battery))
				logger->log("Battery status received.");
			else
				logger->log("Error getting battery status.");

			logger->log("Trying to send second SMS...");
			for (int i = 0; GPS::get_instance().get_PDOP() > 5 && i < 5; i++)
				this_thread::sleep_for(500ms);

			if ( ! GSM::get_instance().send_SMS(
				"Alt: "+ to_string((int) GPS::get_instance().get_altitude()) +
				" m\r\nLat: "+ to_string(GPS::get_instance().get_latitude()) +"\r\n"+
				"Lon: "+ to_string(GPS::get_instance().get_longitude()) +"\r\n"+
				(bat_status ? "Main bat: "+ to_string((int) (main_battery*100)) +"%\r\n"+
					"GSM bat: "+ to_string((int) (gsm_battery*100)) +"%\r\n" : "Bat: ERR\r\n") +
				"Fix: "+ (GPS::get_instance().is_fixed() ? "OK" : "ERR") +
				"\r\nSat: "+ to_string(GPS::get_instance().get_satellites()), SMS_PHONE))
			{
				logger->log("Error sending second SMS.");
			}
			else
			{
				logger->log("Second SMS sent.");
			}

			bat_status = false;
		}
	}

	if ( ! wait_down_for(500))
	{
		logger->log("500 m mark passed going down.");

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

			logger->log("Getting battery values...");
			if (bat_status = GSM::get_instance().get_battery_status(main_battery, gsm_battery))
				logger->log("Battery status received.");
			else
				logger->log("Error getting battery status.");

			logger->log("Trying to send third SMS...");
			for (int i = 0; GPS::get_instance().get_PDOP() > 5 && i < 5; i++)
				this_thread::sleep_for(500ms);

			if ( ! GSM::get_instance().send_SMS(
				"Alt: "+ to_string((int) GPS::get_instance().get_altitude()) +
				" m\r\nLat: "+ to_string(GPS::get_instance().get_latitude()) +"\r\n"+
				"Lon: "+ to_string(GPS::get_instance().get_longitude()) +"\r\n"+
				(bat_status ? "Main bat: "+ to_string((int) (main_battery*100)) +"%\r\n"+
					"GSM bat: "+ to_string((int) (gsm_battery*100)) +"%\r\n" : "Bat: ERR\r\n") +
				"Fix: "+ (GPS::get_instance().is_fixed() ? "OK" : "ERR") +
				"\r\nSat: "+ to_string(GPS::get_instance().get_satellites()), SMS_PHONE))
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

	double main_battery = 0, gsm_battery = 0;
	bool bat_status = false;

	logger->log("Getting battery values...");
	if (bat_status = (GSM::get_instance().get_battery_status(main_battery, gsm_battery) ||
						GSM::get_instance().get_battery_status(main_battery, gsm_battery)))
		logger->log("Battery status received.");
	else
		logger->log("Error getting battery status.");

	logger->log("Sending landed SMS...");
	for (int i = 0; GPS::get_instance().get_PDOP() > 5 && i < 5; i++)
		this_thread::sleep_for(500ms);

	if ( ! GSM::get_instance().send_SMS(
		"Landed\r\nAlt: "+ to_string((int) GPS::get_instance().get_altitude()) +
		" m\r\nLat: "+ to_string(GPS::get_instance().get_latitude()) +"\r\n"+
		"Lon: "+ to_string(GPS::get_instance().get_longitude()) +"\r\n"+
		(bat_status ? "Main bat: "+ to_string((int) (main_battery*100)) +"%\r\n"+
			"GSM bat: "+ to_string((int) (gsm_battery*100)) +"%\r\n" : "Bat: ERR\r\n") +
		"Fix: "+ (GPS::get_instance().is_fixed() ? "OK" : "ERR") +
		"\r\nSat: "+ to_string(GPS::get_instance().get_satellites()), SMS_PHONE))
	{
		logger->log("Error sending landed SMS. Trying again in 10 minutes...");
	}
	else
	{
		logger->log("Landed SMS sent. Sending backup SMS in 10 minutes...");
	}

	this_thread::sleep_for(10min);

	logger->log("Getting battery values...");
	if (bat_status = (GSM::get_instance().get_battery_status(main_battery, gsm_battery) ||
						GSM::get_instance().get_battery_status(main_battery, gsm_battery)))
		logger->log("Battery status received.");
	else
		logger->log("Error getting battery status.");

	logger->log("Sending second landed SMS...");
	while (GPS::get_instance().get_PDOP() > 5)
		this_thread::sleep_for(500ms);

	while (( ! GSM::get_instance().send_SMS(
		"Landed\r\nAlt: "+ to_string((int) GPS::get_instance().get_altitude()) +
		" m\r\nLat: "+ to_string(GPS::get_instance().get_latitude()) +"\r\n"+
		"Lon: "+ to_string(GPS::get_instance().get_longitude()) +"\r\n"+
		(bat_status ? "Main bat: "+ to_string((int) (main_battery*100)) +"%\r\n"+
			"GSM bat: "+ to_string((int) (gsm_battery*100)) +"%\r\n" : "Bat: ERR\r\n") +
		"Fix: "+ (GPS::get_instance().is_fixed() ? "OK" : "ERR") +
		"\r\nSat: "+ to_string(GPS::get_instance().get_satellites()), SMS_PHONE) ||
		! GPS::get_instance().is_fixed()) &&
		(main_battery >= 0 || main_battery < -1) && gsm_battery >= 0)
	{
		logger->log("Error sending second SMS or GPS without fix, trying again in 5 minutes.");
		this_thread::sleep_for(5min);
		GSM::get_instance().get_battery_status(main_battery, gsm_battery);
	}

	if ((main_battery < 0 && main_battery > -1) || gsm_battery < 0)
	{
		logger->log("Not enough battery.");
		logger->log("Main battery: "+ to_string(main_battery*100) +
			"% - GSM battery: "+ to_string(gsm_battery*100) +"%");
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
}
