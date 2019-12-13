//#define	TRACE

extern "C" {
#include 	"esp_log.h"
}

#include	"config.h"
#include	"debug.h"
#include	"utility.h"

#include	"SenseBuffer.h"

#include	"BME280.h"
#include	"BME280_Temp.h"

#define	TAG	"BME280_Temp"

void
BME280_Temp::get(
	SenseBuffer *buff)
{
ENTER_FUNC;
	int		adc_T;

	int32_t		t_fine
		,	ivar1
		,	ivar2;
	uint8_t		tbuff[3];
	float		mTemp;

	mI2C.read(VAL_TEMPERATURE, (byte *)tbuff, 3);
	err_code = mI2C.err_code;

	if(err_code == ESP_OK) {
		adc_T = ( tbuff[0] * 4096 ) + ( tbuff[1] * 16 ) + ( ( tbuff[2] & 0xF0 ) >> 4 );

		ivar1 = (((( adc_T >> 3 ) - ((int32_t)dig_T1 << 1 ))) * (int32_t)dig_T2) >> 11;
		ivar2 = (((((adc_T >> 4) - (int32_t)dig_T1) * ((adc_T >> 4) - (int32_t)dig_T1)) >> 12) *
			 (int32_t)dig_T3) >> 14;
		t_fine = ivar1 + ivar2;

		mTemp  = ((t_fine * 5 + 128) >> 8) / 100.0f;

		ESP_LOGI(TAG, "temp = %s", ftos(mTemp, 0, 1));
	}else{
		mTemp  = FLT_MAX;
		ESP_LOGE(TAG, "err_code:%d", err_code);
	}

	buff->put(&mTemp, sizeof(float));

LEAVE_FUNC;
}

size_t
BME280_Temp::build(
	char	*p,
	SenseBuffer	*buff)
{
	float	data;

ENTER_FUNC;
	buff->get(&data, sizeof(float));
LEAVE_FUNC;
	return	sprintf(p, "\"value\":%.2f,\"unit\":\"%s\"", data, unit());
}

const	char	*
BME280_Temp::name(void)
{
	return	"BME-280_Temp";
}

const	char	*
BME280_Temp::unit(void)
{
	return	"degree Celsius";
}

const	char	*
BME280_Temp::data_class_name(void)
{
	return	"AirTemperature";
}

