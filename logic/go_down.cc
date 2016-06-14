#if !defined SIM && !defined REAL_SIM
	double start_alt = GPS::get_instance().get_altitude();
#endif

#if defined SIM || defined REAL_SIM
	if (FLIGHT_MAX_HEIGHT > 25000)
#else
	if (start_alt > 25000)
#endif
{
	wait_down_for(25000);
	logger->log("25 km mark passed going down.");
}

#if defined SIM || defined REAL_SIM
	if (FLIGHT_MAX_HEIGHT > 15000)
#else
	if (start_alt > 15000)
#endif
{
	wait_down_for(15000);
	logger->log("15 km mark passed going down.");
}

#if defined SIM || defined REAL_SIM
	if (FLIGHT_MAX_HEIGHT > 5000)
#else
	if (start_alt > 5000)
#endif
{
	wait_down_for(5000);
	logger->log("5 km mark passed going down.");
}

#if defined SIM || defined REAL_SIM
	if (FLIGHT_MAX_HEIGHT > 2000)
#else
	if (start_alt > 2000)
#endif
{
	wait_down_for(2000);
	logger->log("2 km mark passed going down.");
}

logger->log("Turning on GSM...");
GSM::get_instance().turn_on();

double main_battery = 0, gsm_battery = 0;
bool bat_status = false;

logger->log("Waiting for GSM connectivity...");
int count = 0;
while ( ! GSM::get_instance().has_connectivity())
{
	if (count == 20)
	{
		break;
	}

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
	if ((bat_status =
			GSM::get_instance().get_battery_status(main_battery, gsm_battery)))
	{
		logger->log("Battery status received.");
	}
	else
	{
		logger->log("Error getting battery status.");
	}

	logger->log("Trying to send first SMS...");
	for (int i = 0;
		i < 5 && ( ! GPS::get_instance().is_fixed() || GPS::get_instance().get_PDOP() > MIN_DOP);
		++i)
	{
		this_thread::sleep_for(500ms);
	}

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
		if (count == 20)
		{
			break;
		}

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
		if ((bat_status = GSM::get_instance().get_battery_status(main_battery, gsm_battery)))
		{
			logger->log("Battery status received.");
		}
		else
		{
			logger->log("Error getting battery status.");
		}

		logger->log("Trying to send second SMS...");
		for (int i = 0;
			i < 5 && ( ! GPS::get_instance().is_fixed() || GPS::get_instance().get_PDOP() > MIN_DOP);
			++i)
		{
			this_thread::sleep_for(500ms);
		}

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
		if (count > 15)
		{
			break;
		}

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
		if ((bat_status = GSM::get_instance().get_battery_status(main_battery, gsm_battery)))
		{
			logger->log("Battery status received.");
		}
		else
		{
			logger->log("Error getting battery status.");
		}

		logger->log("Trying to send third SMS...");
		for (int i = 0;
			i < 5 && ( ! GPS::get_instance().is_fixed() || GPS::get_instance().get_PDOP() > MIN_DOP);
			++i)
		{
			this_thread::sleep_for(500ms);
		}

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
