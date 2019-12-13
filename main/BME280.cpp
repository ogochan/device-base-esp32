#define	TRACE

extern "C" {
#include 	"driver/i2c.h"
#include 	"esp_log.h"
}

#include	"config.h"
#include	"debug.h"

#include	"BME280.h"

#define	TAG	"BME280"

// static members declaration with initial value

int	BME280::dig_T1 = 0, BME280::dig_T2 = 0, BME280::dig_T3 = 0;
int	BME280::dig_P1 = 0, BME280::dig_P2 = 0, BME280::dig_P3 = 0,
	BME280::dig_P4 = 0, BME280::dig_P5 = 0, BME280::dig_P6 = 0,
	BME280::dig_P7 = 0, BME280::dig_P8 = 0, BME280::dig_P9 = 0;
int	BME280::dig_H1 = 0, BME280::dig_H2 = 0, BME280::dig_H3 = 0,
	BME280::dig_H4 = 0, BME280::dig_H5 = 0, BME280::dig_H6 = 0;

const	uint8_t	BME280::precision = 0;

BME280::BME280()
{
	initialize();
}

void
BME280::initialize(void)
{
ENTER_FUNC;
	err_code = 0;
	byte getvalid;

	getvalid = mI2C.readByte(BME280_START);
	err_code = mI2C.err_code;
	ESP_LOGI(TAG, "BME280_START:%02x", getvalid);

	if(err_code == ESP_OK && getvalid == 0x60 )	{
		fValid = true;
		mI2C.writeByte(0xF2, 1 );
		mI2C.writeByte(0xF4, ( ( 1 << 5 ) | ( 1 << 2 ) | 3 ));
		mI2C.writeByte(0xF5, ( ( 5 << 5 ) | ( 0 << 2 ) | 0 ));
		dig_T1 = (uint16_t)mI2C.readWord(0x88);
		dig_T2 = (int16_t)mI2C.readInt16(0x8A);
		dig_T3 = (int16_t)mI2C.readInt16(0x8C);
		dig_P1 = (uint16_t)mI2C.readWord(0x8E);
		dig_P2 = (int16_t)mI2C.readInt16(0x90);
		dig_P3 = (int16_t)mI2C.readInt16(0x92);
		dig_P4 = (int16_t)mI2C.readInt16(0x94);
		dig_P5 = (int16_t)mI2C.readInt16(0x96);
		dig_P6 = (int16_t)mI2C.readInt16(0x98);
		dig_P7 = (int16_t)mI2C.readInt16(0x9A);
		dig_P8 = (int16_t)mI2C.readInt16(0x9C);
		dig_P9 = (int16_t)mI2C.readInt16(0x9E);
		dig_H1 = (int8_t)mI2C.readByte(0xA1);
		dig_H2 = (int16_t)mI2C.readInt16(0xE1);
		dig_H3 = (int8_t)mI2C.readByte(0xE3);
		dig_H4 = ( (int16_t)mI2C.readByte(0xE4) << 4 ) | ( 0x0F & (int16_t)mI2C.readByte(0xE5) );
		dig_H5 = ( (int16_t)mI2C.readByte(0xE6) << 4 ) | ( 0x0F & ( (int16_t)mI2C.readByte(0xE7) >> 4 ) );
		dig_H6 = (int8_t)mI2C.readByte(0xE8);
		ESP_LOGI(TAG, "valid");
	} else {
		fValid = false;
		ESP_LOGE(TAG, "invalid");

	}
LEAVE_FUNC;
}


BME280::~BME280()
{
}


