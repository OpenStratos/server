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

logger->log("Setting GPS to airborne mode...");
GPS::get_instance().notify_takeoff();
