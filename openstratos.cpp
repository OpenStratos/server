#include "openstratos.hpp"

int main(void)
{
	cout << "[OpenStratos] Starting" << endl;
	wiringPiSetupSys();

	cout << "[OpenStratos] Sleeping for 10s before running checks" << endl;
	delay(10000);

	cout << "[OpenStratos] Starting checks" << endl;
	return 0;
}