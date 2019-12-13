#ifndef	__INC_SENSE_BUFFER_H__
#define	__INC_SENSE_BUFFER_H__

#include	<stdlib.h>
#include	<stdint.h>
#include	<string.h>
#include	"config.h"

class	SenseBuffer	{
  public:
	void	put(void *data, size_t size)	{
		memcpy(_w_pointer, data, size);
		_w_pointer += size;
	};
	void	get(void *data, size_t size)	{
		memcpy(data, _r_pointer, size);
		_r_pointer += size;
	};
	void	set_value(uint8_t data)	{
		memcpy(_w_pointer, &data, 1);
		_w_pointer ++;
	};
	uint8_t	current_value(void)	{
		return	(*_r_pointer);
	};
	uint8_t	get_value(void)	{
		uint8_t	ret = *_r_pointer;
		_r_pointer ++;
		return	(ret);
	};
	int		used_size(void)	{
		return	(int)(_w_pointer - _buff);
	};
	void	rewind_read(void)	{
		_r_pointer = _buff;
	}
	void	rewind_write(void)	{
		_w_pointer = _buff;
	}
	void	rewind_read(uint8_t *p)	{
		_r_pointer = p;
	}
	uint8_t	*mark_read(void)	{
		return	(_r_pointer);
	}
	bool	is_end(void)	{
		return	(_r_pointer < _w_pointer) ? false : true;
	}
	uint8_t	*buff(void)	{
		return	(_buff);
	}
	uint8_t	*pointer(void)	{
		return	(_w_pointer);
	}
	void	set_pointer(uint32_t pointer)	{
		_w_pointer = (uint8_t *)pointer;
	}
	void	check_space(void);
	SenseBuffer(size_t size)	{
		_buff = (uint8_t *)malloc(SIZE_OF_SENSOR_BUFF);
	}
	~SenseBuffer(void)	{
		free(_buff);
	}

  protected:
	uint8_t	*_buff;
	uint8_t	*_w_pointer;
	uint8_t	*_r_pointer;
};

#endif
