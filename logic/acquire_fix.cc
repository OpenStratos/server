while ( ! GPS::get_instance().is_fixed())
	this_thread::sleep_for(1s);

logger->log("GPS fix acquired, waiting 10 seconds for stabilization and date change.");
this_thread::sleep_for(10s);
