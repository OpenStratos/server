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

logger->log("Trying to send launch confirmation SMS...");
for (int i = 0; GPS::get_instance().get_PDOP() > 5 && i < 5; i++)
{
	this_thread::sleep_for(500ms);
}

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
{
	this_thread::sleep_for(500ms);
}
double maximum_altitude = GPS::get_instance().get_altitude();

wait_up_for(1200, maximum_altitude);
logger->log("1.2 km mark.");

logger->log("Getting battery values...");
if ((bat_status = GSM::get_instance().get_battery_status(main_battery, gsm_battery)))
{
	logger->log("Battery status received.");
}
else
{
	logger->log("Error getting battery status.");
}

logger->log("Trying to send \"going up\" SMS...");
for (int i = 0; GPS::get_instance().get_PDOP() > 5 && i < 5; i++)
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

bool bursted = wait_up_for(5000, maximum_altitude);
if ( ! bursted)
{
	logger->log("5 km mark passed going up.");
}
else
{
	*state = set_state(GOING_DOWN);
	logger->log("State changed to "+ state_to_string(*state) +".");
	break;
}

bursted = wait_up_for(10000, maximum_altitude);
if ( ! bursted)
{
	logger->log("10 km mark passed going up.");
}
else
{
	*state = set_state(GOING_DOWN);
	logger->log("State changed to "+ state_to_string(*state) +".");
	break;
}

bursted = wait_up_for(15000, maximum_altitude);
if ( ! bursted)
{
	logger->log("15 km mark passed going up.");
}
else
{
	*state = set_state(GOING_DOWN);
	logger->log("State changed to "+ state_to_string(*state) +".");
	break;
}

bursted = wait_up_for(20000, maximum_altitude);
if ( ! bursted)
{
	logger->log("20 km mark passed going up.");
}
else
{
	*state = set_state(GOING_DOWN);
	logger->log("State changed to "+ state_to_string(*state) +".");
	break;
}

bursted = wait_up_for(25000, maximum_altitude);
if ( ! bursted)
{
	logger->log("25 km mark passed going up.");
}
else
{
	*state = set_state(GOING_DOWN);
	logger->log("State changed to "+ state_to_string(*state) +".");
	break;
}

bursted = wait_up_for(30000, maximum_altitude);
if ( ! bursted)
{
	logger->log("30 km mark passed going up.");
}
else
{
	*state = set_state(GOING_DOWN);
	logger->log("State changed to "+ state_to_string(*state) +".");
	break;
}

bursted = wait_up_for(35000, maximum_altitude);
if ( ! bursted)
{
	logger->log("35 km mark passed going up.");
}
else
{
	*state = set_state(GOING_DOWN);
	logger->log("State changed to "+ state_to_string(*state) +".");
	break;
}

while ( ! has_bursted(maximum_altitude))
{
	double current_altitude;
	for (int i = 0; GPS::get_instance().get_VDOP() > 5 && i < 5; i++)
	{
		this_thread::sleep_for(500ms);
	}

	if ((current_altitude = GPS::get_instance().get_altitude()) > maximum_altitude)
	{
		maximum_altitude = current_altitude;
	}
}

logger->log("Balloon burst at about "+ to_string((int) maximum_altitude) +" m.");
