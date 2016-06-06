while ( ! GPS::get_instance().is_fixed())
	this_thread::sleep_for(1s);

logger->log("GPS fix acquired, waiting 2 seconds for stabilization.");
this_thread::sleep_for(2s);
