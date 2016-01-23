#include "logic/logic.h"

void os::shut_down(Logger* logger)
{
	logger->log("Shutting down...");

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

	logger->log("Powering off...");
}
