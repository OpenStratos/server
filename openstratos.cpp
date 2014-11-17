#include "openstratos.hpp"

int main(void)
{
	printf("[OpenStratos] Starting\n");
	wiringPiSetupSys();

	printf("[OpenStratos] Sleeping for 10s before running checks\n");
	delay(10000);

	printf("[OpenStratos] Starting checks\n");
	return 0;
}