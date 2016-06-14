State last_state = get_last_state();
State state = set_state(SAFE_MODE);

int count = 0;
double latitude = 0, longitude = 0;

check_or_create("data/logs");
check_or_create("data/logs/main");
check_or_create("data/logs/system");
check_or_create("data/logs/camera");
check_or_create("data/logs/GPS");
check_or_create("data/logs/GSM");

struct timeval timer;
gettimeofday(&timer, NULL);
struct tm* now = gmtime(&timer.tv_sec);

Logger logger("data/logs/main/OpenStratos."+ to_string(now->tm_year+1900) +"-"+
	to_string(now->tm_mon+1) +"-"+ to_string(now->tm_mday) +"."+ to_string(now->tm_hour)
	+"-"+ to_string(now->tm_min) +"-"+ to_string(now->tm_sec) +".log", "OpenStratos");

logger.log(PACKAGE_STRING);
logger.log("Safe mode.");
logger.log("Last state was "+state_to_string(last_state)+".");

switch (last_state)
{
	case INITIALIZING:
	case ACQUIRING_FIX:
	{
		logger.log("OpenStratos did not send the initialization SMS.");
		remove(STATE_FILE);
		logger.log("Rebooting after deleting the state file to start again.");
		#ifndef NO_POWER_OFF
			sync();
			reboot(RB_AUTOBOOT);
		#else
			exit(0);
		#endif
	break;
	}
	case FIX_ACQUIRED:
	// It could be that the SMS was sent but the state didn't change
	case WAITING_LAUNCH:
	case GOING_UP:
	case GOING_DOWN:
	case LANDED:
	{
		logger.log("OpenStratos might have been launched, trying to recover.");

		logger.log("Initializing WiringPi...");
		wiringPiSetup();
		logger.log("WiringPi initialized.");

		logger.log("Initializing GPS...");
		while ( ! GPS::get_instance().initialize() && ++count < 5)
		{
			logger.log("GPS initialization error.");
		}

		if (count < 5)
		{
			logger.log("GPS initialized.");
			count = 0;
			logger.log("Waiting for GPS fix...");
			while ( ! GPS::get_instance().is_fixed() && ++count < 100)
			{
				this_thread::sleep_for(10s);
			}

			if (count == 100)
			{
				logger.log("Not getting fix. Going to recovery mode.");
				#ifndef NO_POWER_OFF
					sync();
					reboot(RB_AUTOBOOT);
				#else
					exit(1);
				#endif
			}

			logger.log("GPS fix acquired. Sleeping one minute for stability.");
			this_thread::sleep_for(1min);
			logger.log("Getting real state from GPS information.");
			state = (state == LANDED) ? LANDED : get_real_state();
			logger.log("Seems that the current state is "+state_to_string(state)+".");

			logger.log("Initializing GSM...");
			if ( ! GSM::get_instance().initialize())
			{
				logger.log("GSM initialization error. Going to recovery mode.");
				#ifndef NO_POWER_OFF
					sync();
					reboot(RB_AUTOBOOT);
				#else
					exit(1);
				#endif
			}
			logger.log("GSM initialized.");

			if (state != LANDED)
			{
				logger.log("Trying to start recording...");
				if (Camera::get_instance().record())
				{
					logger.log("Recording.");
				}
				else
				{
					logger.log("Error starting recording");
				}
			}

			logger.log("Going to the main while.");
			main_while(&logger, &state);
			shut_down(&logger);
		}
		else
		{
			logger.log("Error initializing GPS. Going to recovery mode.");
			#ifndef NO_POWER_OFF
				sync();
				reboot(RB_AUTOBOOT);
			#else
				exit(1);
			#endif
		}
	break;
	}
	case SHUT_DOWN:
	{
		logger.log("Seems OpenStratos was shutting down when the error occurred.");
		shut_down(&logger);
	break;
	}
	case SAFE_MODE:
	{
		logger.log("Recovery mode");

		logger.log("Initializing WiringPi...");
		wiringPiSetup();
		logger.log("WiringPi initialized.");

		logger.log("Initializing GSM...");
		while ( ! GSM::get_instance().initialize())
		{
			logger.log("GSM initialization error.");
		}
		logger.log("GSM initialized");
		logger.log("Waiting for GSM connectivity...");
		while ( ! GSM::get_instance().has_connectivity())
		{
			this_thread::sleep_for(5s);
		}
		logger.log("GSM connected.");

		logger.log("Sending mayday messages...");
		for (count = 0; count < 2;)
		{
			this_thread::sleep_for(5min);

			GSM::get_instance().get_location(latitude, longitude);
			GSM::get_instance().send_SMS("MAYDAY\r\nLat: "+ to_string(latitude) +"\r\n"+
				"Lon: "+ to_string(longitude), SMS_PHONE) && ++count;
		}
		logger.log("Mayday messages sent.");

		logger.log("Initializing GPS...");
		count = 0;
		while ( ! GPS::get_instance().initialize() && ++count < 5)
		{
			logger.log("GPS initialization error.");
		}

		if (count < 5)
		{
			logger.log("GPS initialized.");
			count = 0;
			logger.log("Waiting for GPS fix...");
			while ( ! GPS::get_instance().is_fixed() && ++count < 100)
			{
				this_thread::sleep_for(10s);
			}

			if (count == 100)
			{
				logger.log("Not getting fix.");
				shut_down(&logger);
			}

			logger.log("GPS fix acquired.");

			logger.log("Notifying GPS about entering safe mode...");
			GPS::get_instance().notify_safe_mode();

			this_thread::sleep_for(5s);
			for (int i = 0;
				GPS::get_instance().get_HDOP() > 5 && i < 10;
				++i)
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

			main_while(&logger, &state);
			shut_down(&logger);
		}
		else
		{
			shut_down(&logger);
		}
	}
}
