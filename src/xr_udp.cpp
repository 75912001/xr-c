#include "xr_udp.h"

namespace xr{
int udp_t::send( const void* buf, int len )
{
	return (int)HANDLE_EINTR(::sendto(this->fd, buf, len, MSG_NOSIGNAL, (sockaddr*)&(this->addr), sizeof(this->addr)));
}

int udp_t::recv( void* data, int len )
{
	//todo 
	return 0;
}

int udp_t::connect( const char* ip, uint16_t port )
{
	::memset(&(this->addr), 0, sizeof(this->addr));
	this->addr.sin_family = AF_INET;
	this->addr.sin_port = htons(port);
	::inet_pton(AF_INET, ip, &(this->addr.sin_addr));
	this->fd = ::socket(AF_INET, SOCK_DGRAM, 0);
	return this->fd;
}
}//end namespace xr