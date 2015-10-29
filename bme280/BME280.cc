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

	float T = (t_fine * 5 + 128) >> 8;
	return T/100;
}

double BME280::get_humidity() const
{
	// TODO
}

double BME280::get_pressure() const
{
	// TODO
}
