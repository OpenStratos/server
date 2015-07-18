#ifndef GSM_GMS_H_
#define GSM_GSM_H_

#include <string>

#include "serial/Serial.h"

using namespace std;

namespace os {

	class GSM
	{
	private:
		Serial serial;

		int pwr_gpio;
		int status_gpio;
		int fh;

		GSM() = default;
		~GSM();

		const string send_command_read(const string& command) const;
		bool init_GPRS() const;
		bool tear_down_GPRS() const;
	public:
		GSM(GSM& copy) = delete;
		static GSM& get_instance();

		bool initialize(int pwr_gpio, int status_gpio, const string& serial_URL);
		bool send_SMS(const string& message, const string& number) const;
		bool get_location(double& latitude, double& longitude) const;
		bool get_status() const;
		bool is_up() const;
		bool turn_on() const;
		bool turn_off() const;
	};
}
#endif
