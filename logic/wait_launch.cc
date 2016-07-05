logger->log("Waiting for launch...");

for (int i = 0; i < 30 && ( ! GPS::get_instance().is_fixed() ||
				GPS::get_instance().get_VDOP() > MAX_DOP); ++i)
{
	this_thread::sleep_for(500ms);
}

if (GPS::get_instance().get_VDOP() > MAX_DOP)
{
	logger->log("Launch altitude with bad precission, VDOP: " + to_string_prec(GPS::get_instance().get_VDOP(), 2));
}
launch_altitude = GPS::get_instance().get_altitude();
logger->log("Launch altitude considered " + to_string((int) launch_altitude) + "m");

#ifdef SIM
	this_thread::sleep_for(2min);
#endif
#ifdef REAL_SIM
	this_thread::sleep_for(15min);
#endif

while ( ! has_launched(launch_altitude))
	this_thread::sleep_for(1s);

logger->log("Balloon launched.");
logger->log("Current altitude: "+ to_string((int) GPS::get_instance().get_altitude()) + "m");
