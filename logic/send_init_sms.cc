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

logger->log("Waiting for GPS PDOP < "+to_string(MIN_DOP)+"...");
while ( ! GPS::get_instance().is_fixed() || GPS::get_instance().get_PDOP() > MIN_DOP)
{
	this_thread::sleep_for(1s);
}

logger->log("Sending initialization SMS...");
if ( ! GSM::get_instance().send_SMS(
	"Init: OK.\r\nAlt: "+ to_string((int) GPS::get_instance().get_altitude()) +
	" m\r\nLat: "+ to_string(GPS::get_instance().get_latitude()) +
	"\r\nLon: "+ to_string(GPS::get_instance().get_longitude()) +"\r\n"+
	"\r\nPDOP: "+to_string(GPS::get_instance().get_PDOP()) +
	"\r\nSat: "+ to_string(GPS::get_instance().get_satellites()) +
	"\r\nFix: "+ (GPS::get_instance().is_fixed() ? "OK" : "ERR") +
	(bat_status ? "\r\nMain bat: "+ to_string((int) (main_battery*100)) +
		"%\r\nGSM bat: "+ to_string((int) (gsm_battery*100)) +"%" : "\r\nBat: ERR") +
	"\r\nWaiting launch.", SMS_PHONE))
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
