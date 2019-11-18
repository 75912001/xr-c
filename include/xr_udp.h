#pragma once

#include "xr_net.h"

namespace xr
{
class udp_t : public net_t
{
protected:
	sockaddr_in addr;

public:
	udp_t()
	{
		::memset(&(this->addr), 0, sizeof(this->addr));
	}
	virtual int send(const void *data, int len);
	virtual int recv(void *data, int len);
	int connect(const char *ip, uint16_t port);
};
} //end namespace xr
