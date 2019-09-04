#ifndef __INC_CONFIG_COMMON_H__
#define __INC_CONFIG_COMMON_H__

#define	GPIO_SDA			GPIO_NUM_17
#define	GPIO_SCL			GPIO_NUM_16

#define	O2_RX_PIN			GPIO_NUM_26
#define O2_TX_PIN			GPIO_NUM_27
#define UART_NUM			UART_NUM_1

#define	CONNECTION_TIMEOUT		30

#define	UART_TIMEOUT			30

#define I2C_MASTER_FREQ_HZ		10000

#define	NUMBER_OF_SENSORS		10

#define	strlcmp(p,q)			strncmp((p),(q),strlen(q))
#define	memclear(buff,size)		memset((buff),0,(size))

#endif // __CONFIG_COMMON_H__

