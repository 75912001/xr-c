#pragma once

#include "xr_util.h"
#include "xr_file.h"

namespace xr
{
class net_t
{
public:
	net_t()
	{
		this->fd = INVALID_FD;
	}
	virtual ~net_t()
	{
		file_t::close_fd(this->fd);
	}
	int fd;

public:
	//send `total` bytes of data
	//return:number of bytes sent:success
	//-1:error
	//负数:其他错误
	virtual int send(const void *data, int len) = 0;

	//receive data
	//data:buffer to hold the receiving data
	//len:size of `data`
	//return:number of bytes receive:success
	//0:on connection closed by peer
	//-1:error (no handle EAGAIN)
	//负数:其他错误
	virtual int recv(void *data, int len) = 0;
	virtual bool is_connect()
	{
		return INVALID_FD != this->fd;
	}
};
} //end namespace xr
