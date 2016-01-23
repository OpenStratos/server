logger->log("Starting video recording...");
if ( ! Camera::get_instance().record())
{
	logger->log("Error starting recording");
	logger->log("Turning GSM off...");
	if (GSM::get_instance().turn_off())
		logger->log("GSM off.");
	else
		logger->log("Error turning GSM off.");

	logger->log("Turning GPS off...");
	if (GPS::get_instance().turn_off())
		logger->log("GPS off.");
	else
		logger->log("Error turning GPS off.");

	#ifndef NO_POWER_OFF
		sync();
		reboot(RB_POWER_OFF);
	#else
		exit(1);
	#endif
}
logger->log("Recording started.");
