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
