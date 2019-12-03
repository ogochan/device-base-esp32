#ifndef	__INC_I2C_H__
#define	__INC_I2C_H__

#include	<stdint.h>

extern "C" {
#include 	"esp_err.h"
#include	"types.h"
}

extern	esp_err_t	initialize_i2c(void);

class I2C
{
  public:
	I2C(int slaveAddress);
	~I2C();


	void	i2c_slave_write(byte address,byte *buf,int size);
	void	i2c_slave_read(byte address,byte command,byte *data,int size);

	void	write_low(byte *buf,int size);
	void	read_low(byte *data,int size);

	void	write(byte command, byte* data, int nSize);
	void	read(byte command, byte* data, int nSize);

	void	writeByte(byte command, byte data);
	byte	readByte(byte command);
	void	writeWord(byte command, uint16_t data);
	uint16_t	readWord(byte command);
	int16_t	readInt16(byte command);

	esp_err_t	err_code;

  protected:
	byte 	address;

};

#endif
