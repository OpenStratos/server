#ifndef ADC_MCP3424_H_
#define ADC_MCP3424_H_

namespace os {

	class MCP3424
	{
	private:
		int fd;
		int gain;
		int res;

		inline int get_divisor() {return 1 << (this->gain + 2*this->res);}

	public:
		MCP3424(int i2c_address, int gain, int res);
		MCP3424(MCP3424& copy) = delete;
		~MCP3424() = default;

		double read_channel(int channel);
	};
}

#endif // ADC_MCP3424_H_
