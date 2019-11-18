//各种内存类型

#pragma once

#include "xr_util.h"

namespace xr
{
//自动无限放大,弹出全部数据后会释放内存空间.
//单线程操作
class active_buf_t
{
public:
	unsigned char *data; //数据头指针
	uint32_t total_len;  //已分配的总长度
	uint32_t write_pos;  //已使用到的位置
	active_buf_t()
	{
		this->init();
	}
	virtual ~active_buf_t()
	{
		this->clear();
	}

	//写数据
	//pdata:数据
	//len:数据长度
	inline void write(const void *pdata, uint32_t len)
	{
		uint32_t remain_len = this->total_len - this->write_pos;
		if (remain_len >= len)
		{
			::memcpy(this->data + this->write_pos, pdata, len);
			this->write_pos += len;
		}
		else
		{
			this->data = (unsigned char *)::realloc(this->data, len + this->write_pos);
			::memcpy(this->data + this->write_pos, pdata, len);
			this->total_len = len + this->write_pos;
			this->write_pos = this->total_len;
		}
	}

	//弹出长度为len的数据
	//returns:剩余的数据长度(0:没有剩余)
	//len:弹出长度
	inline uint32_t pop(uint32_t len)
	{
		if (this->write_pos <= len)
		{
			//全部弹出
			this->clear();
		}
		else
		{
			::memmove(this->data, this->data + len, this->write_pos - len);
			this->write_pos -= len;
		}
		return this->write_pos;
	}
	//清理所有数据/释放内存空间
	inline void clear()
	{
		SAFE_FREE(this->data);
		this->init();
	}

private:
	inline void init()
	{
		this->data = NULL;
		this->total_len = 0;
		this->write_pos = 0;
	}
};

//对象内存todo

class object_buf_t
{
};
} //end namespace xr