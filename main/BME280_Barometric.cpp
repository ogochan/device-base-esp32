#define	TRACE

extern "C" {
#include 	"esp_log.h"
}

#include	"config.h"
#include	"debug.h"
#include	"utility.h"

#include	"SenseBuffer.h"

#include	"BME280.h"
#include	"BME280_Barometric.h"


#define	TAG	"BME280_Barometric"

void
BME280_Barometric::get(
	SenseBuffer	*buff)
{
ENTER_FUNC;
	int		adc_T
		,	adc_P;

	int32_t		t_fine
		,	ivar1
		,	ivar2;
	int32_t		var1
		,	var2;
	uint32_t	p;
	uint8_t		tbuff[3]
		,	pbuff[3];
	float		mPressure;

	mI2C.read(VAL_PRESSURE, (byte *)tbuff, 3);
	err_code = mI2C.err_code;
	
	if(err_code == ESP_OK){
		mI2C.read(VAL_PRESSURE, (byte *)pbuff, 3);
		err_code = mI2C.err_code;

		if(err_code == ESP_OK){

			adc_T = ( tbuff[0] * 4096 ) + ( tbuff[1] * 16 ) + ( ( tbuff[2] & 0xF0 ) >> 4 );
			adc_P = ( pbuff[0] * 4096 ) + ( pbuff[1] * 16 ) + ( ( pbuff[2] & 0xF0 ) >> 4 );

			ivar1 = (((( adc_T >> 3 ) - ((int32_t)dig_T1 << 1 ))) * (int32_t)dig_T2) >> 11;
			ivar2 = (((((adc_T >> 4) - (int32_t)dig_T1) * ((adc_T >> 4) - (int32_t)dig_T1)) >> 12) *
				 (int32_t)dig_T3) >> 14;
			t_fine = ivar1 + ivar2;

			var1 = ( (int32_t)t_fine >> 1 ) - (int32_t)64000;
			var2 = ((( var1 >> 2 ) * ( var1 >> 2 )) >> 11 ) * (int32_t)dig_P6;
			var2 = var2 + ((var1 * (int32_t)dig_P5) << 1 );
			var2 = ( var2 >> 2 ) + ( (int32_t)dig_P4 << 16 );
			var1 = (((dig_P3 * (((var1 >> 2 ) * ( var1 >> 2)) >> 13 )) >> 3) + (((int32_t)dig_P2 * var1) >>1 )) >> 18;
			var1 = (((32768 + var1)) * (int32_t)dig_P1) >> 15;
			if (var1 == 0)	{
				mPressure = 0.0;
			} else {
				p = ((uint32_t)((1048576 - adc_P ) - ( var2 >> 12))) * 3125;
				if (p < 0x80000000)	{
					p = (p << 1) / (uint32_t)var1;
				} else {
					p = (p / (uint32_t)var1) * 2;
				}
				var1 = ((int32_t)dig_P9 * ((int32_t)(( p >> 3 ) * ( p >> 3 )) >> 13)) >> 12;
				var2 = (( (int32_t)p >> 2 ) * (int32_t)dig_P8) >> 13;
				p = (uint32_t)((int32_t)p + ((var1 + var2 + dig_P7) >> 4));
				mPressure = p / 100.0f;
			}
			ESP_LOGI(TAG, "barometric = %s", ftos(mPressure, 0, 0));

		} else {
			mPressure = FLT_MAX;
			ESP_LOGE(TAG, "err_code:%d", err_code);			
		}
	} else {
		mPressure = FLT_MAX;
		ESP_LOGE(TAG, "err_code:%d", err_code);			
	}

	buff->put(&mPressure, sizeof(float));

LEAVE_FUNC;
}

size_t
BME280_Barometric::build(
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
BME280_Barometric::name(void)
{
	return	"BME-280_Barometric";
}

const	char	*
BME280_Barometric::unit(void)
{
	return	"hPa";
}
const	char	*
BME280_Barometric::data_class_name(void)
{
	return	"BarometricPressure";
}

