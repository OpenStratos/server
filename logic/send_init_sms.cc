double main_battery = 0, gsm_battery = 0;
bool bat_status = false;

logger->log("Getting battery values...");
if ((bat_status = GSM::get_instance().get_battery_status(main_battery, gsm_battery)))
{
	logger->log("Battery status received.");
}
else
{
	logger->log("Error getting battery status.");
}

logger->log("Sending initialization SMS...");
while (GPS::get_instance().get_PDOP() > 5)
{
	this_thread::sleep_for(1s);
}

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
	{
		logger->log("GSM off.");
	}
	else
	{
		logger->log("Error turning GSM off.");
	}

	logger->log("Turning GPS off...");
	if (GPS::get_instance().turn_off())
	{
		logger->log("GPS off.");
	}
	else
	{
		logger->log("Error turning GPS off.");
	}

	#ifndef NO_POWER_OFF
		sync();
		reboot(RB_POWER_OFF);
	#else
		exit(1);
	#endif
}
logger->log("Initialization SMS sent.");
