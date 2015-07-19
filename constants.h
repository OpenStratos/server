#ifndef CONSTANTS_H_
	#define CONSTANTS_H_

	#define ERR_OK 0
	#define ERR_CHK 1
	#define ERR_INT 2
	#define ERR_PWD 3

	#define TEMP_VIN 5
	#define TEMP_R 500

	#define BAT_MAX 8.4
	#define BAT_MIN 7.4
	#define BAT_R1 3300
	#define BAT_R2 4700

	#define GSM_LOC_SERV "gprs-service.com"
	#define GSM_UART "/dev/ttyAMA0"
	#define GSM_PWR_GPIO 4
	#define GSM_STATUS_GPIO 5
	#define GSM_BAUDRATE 4800
	#define GSM_ENDL "\r\n"

	#define STATE_FILE "data/last_state.txt"
#endif // CONSTANTS_H_
