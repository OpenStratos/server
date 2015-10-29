#include "bme280/BME280.h"

#include <cstdint>

#include <wiringPiI2C.h>

#include "constants.h"

using namespace os;

BME280::BME280()
{
	this->filehandle = wiringPiI2CSetup(BME280_DEVICE);

	if (wiringPiI2CReadReg8(this->filehandle, BME280_REGISTER_CHIPID) != 0x60)
		return;

	this->bme280_calib.dig_T1 = this->to_LE(wiringPiI2CReadReg16(
									this->filehandle, BME280_REGISTER_DIG_T1));
	this->bme280_calib.dig_T2 = this->to_LE(wiringPiI2CReadReg16(
									this->filehandle, BME280_REGISTER_DIG_T2));
	this->bme280_calib.dig_T3 = this->to_LE(wiringPiI2CReadReg16(
									this->filehandle, BME280_REGISTER_DIG_T3));

	this->bme280_calib.dig_P1 = this->to_LE(wiringPiI2CReadReg16(
									this->filehandle, BME280_REGISTER_DIG_P1));
	this->bme280_calib.dig_P2 = this->to_LE(wiringPiI2CReadReg16(
									this->filehandle, BME280_REGISTER_DIG_P2));
	this->bme280_calib.dig_P3 = this->to_LE(wiringPiI2CReadReg16(
									this->filehandle, BME280_REGISTER_DIG_P3));
	this->bme280_calib.dig_P4 = this->to_LE(wiringPiI2CReadReg16(
									this->filehandle, BME280_REGISTER_DIG_P4));
	this->bme280_calib.dig_P5 = this->to_LE(wiringPiI2CReadReg16(
									this->filehandle, BME280_REGISTER_DIG_P5));
	this->bme280_calib.dig_P6 = this->to_LE(wiringPiI2CReadReg16(
									this->filehandle, BME280_REGISTER_DIG_P6));
	this->bme280_calib.dig_P7 = this->to_LE(wiringPiI2CReadReg16(
									this->filehandle, BME280_REGISTER_DIG_P7));
	this->bme280_calib.dig_P8 = this->to_LE(wiringPiI2CReadReg16(
									this->filehandle, BME280_REGISTER_DIG_P8));
	this->bme280_calib.dig_P9 = this->to_LE(wiringPiI2CReadReg16(
									this->filehandle, BME280_REGISTER_DIG_P9));

	this->bme280_calib.dig_H1 = wiringPiI2CReadReg8(this->filehandle,
									BME280_REGISTER_DIG_H1);
	this->bme280_calib.dig_H2 = this->to_LE(wiringPiI2CReadReg16(
									this->filehandle, BME280_REGISTER_DIG_H2));
	this->bme280_calib.dig_H3 = wiringPiI2CReadReg8(this->filehandle,
									BME280_REGISTER_DIG_H3);
	this->bme280_calib.dig_H4 = (wiringPiI2CReadReg8(this->filehandle,
									BME280_REGISTER_DIG_H4) << 4) |
									(wiringPiI2CReadReg8(this->filehandle,
										BME280_REGISTER_DIG_H4+1) & 0xF);
	this->bme280_calib.dig_H5 = (wiringPiI2CReadReg8(this->filehandle,
									BME280_REGISTER_DIG_H5+1) << 4) |
									(wiringPiI2CReadReg8(this->filehandle,
										BME280_REGISTER_DIG_H5) >> 4);
	this->bme280_calib.dig_H6 = (int8_t) wiringPiI2CReadReg8(this->filehandle,
											BME280_REGISTER_DIG_H6);

	// Set before CONTROL (DS 5.4.3)
	wiringPiI2CWriteReg8(this->filehandle, BME280_REGISTER_CONTROLHUMID, 0x03);
	wiringPiI2CWriteReg8(this->filehandle, BME280_REGISTER_CONTROL, 0x3F);
}

double BME280::get_temperature()
{
	int32_t var1, var2;

	int32_t adc_T = wiringPiI2CReadReg16(this->filehandle,
										BME280_REGISTER_TEMPDATA);
	adc_T <<= 8;
	adc_T |= wiringPiI2CReadReg8(this->filehandle, BME280_REGISTER_TEMPDATA+2);
	adc_T >>= 4;

	var1 = ((((adc_T>>3) - ((int32_t) this->bme280_calib.dig_T1 <<1))) *
			((int32_t) this->bme280_calib.dig_T2)) >> 11;

	var2 = (((((adc_T>>4) - ((int32_t) this->bme280_calib.dig_T1)) *
			((adc_T>>4) - ((int32_t) this->bme280_calib.dig_T1))) >> 12) *
			((int32_t) this->bme280_calib.dig_T3)) >> 14;

	this->t_fine = var1 + var2;

	double T = (t_fine * 5 + 128) >> 8;
	return T/100;
}

double BME280::get_pressure() const
{
	int64_t var1, var2, p;

	int32_t adc_P = wiringPiI2CReadReg16(this->filehandle,
											BME280_REGISTER_PRESSUREDATA);
	adc_P <<= 8;
	adc_P |= wiringPiI2CReadReg8(this->filehandle,
									BME280_REGISTER_PRESSUREDATA+2);
	adc_P >>= 4;

	var1 = ((int64_t) t_fine) - 128000;
	var2 = var1 * var1 * (int64_t) this->bme280_calib.dig_P6;
	var2 = var2 + ((var1*(int64_t) this->bme280_calib.dig_P5)<<17);
	var2 = var2 + (((int64_t) this->bme280_calib.dig_P4)<<35);
	var1 = ((var1 * var1 * (int64_t) this->bme280_calib.dig_P3)>>8) +
			((var1 * (int64_t) this->bme280_calib.dig_P2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*(
				(int64_t) this->bme280_calib.dig_P1)>>33;

	if (var1 == 0) {
		return 0;  // avoid exception caused by division by zero
	}
	p = 1048576 - adc_P;
	p = (((p<<31) - var2)*3125) / var1;
	var1 = (((int64_t) this->bme280_calib.dig_P9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((int64_t) this->bme280_calib.dig_P8) * p) >> 19;

	p = ((p + var1 + var2) >> 8) + (((int64_t) this->bme280_calib.dig_P7)<<4);
	return (double) p/256;
}

double BME280::get_humidity() const
{
	int32_t adc_H = wiringPiI2CReadReg16(this->filehandle,
											BME280_REGISTER_HUMIDDATA);

	int32_t v_x1_u32r;

	v_x1_u32r = (t_fine - ((int32_t)76800));

	v_x1_u32r = (((((adc_H << 14) - (((int32_t) this->bme280_calib.dig_H4) <<
					20) -
					(((int32_t) this->bme280_calib.dig_H5) * v_x1_u32r)) +
						((int32_t)16384)) >> 15) *
						(((((((v_x1_u32r * ((int32_t)
							this->bme280_calib.dig_H6)) >> 10) *
					(((v_x1_u32r * ((int32_t) this->bme280_calib.dig_H3)) >> 11)
						+ ((int32_t)32768))) >> 10) +
						((int32_t)2097152)) * ((int32_t)
							this->bme280_calib.dig_H2) + 8192) >> 14));

	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
					((int32_t) this->bme280_calib.dig_H1)) >> 4));

	v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
	v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
	double h = (v_x1_u32r>>12);
	return  h / 1024;
}
