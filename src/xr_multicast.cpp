#include "xr_multicast.h"
#include "xr_log.h"
#include "xr_net_util.h"

namespace xr{
int mcast_t::create(const char* ip, uint16_t port,
	const char* in_if, const char* out_if)
{
	this->ip = ip;
	this->port = port;
	this->in_if = in_if;
	this->out_if = out_if;

	this->fd = ::socket(AF_INET, SOCK_DGRAM, 0);
	if (INVALID_FD == this->fd) {
		ALERT_LOG("failed to create mcast_fd [err_code:%d, err:%s, %s, %d, %s, %s]", 
		errno, strerror(errno), this->ip.c_str(), this->port, this->in_if.c_str(), this->out_if.c_str());
		return FAIL;
	}
	file_t::set_io_block(this->fd, false);
	::memset(&this->addr, 0, sizeof(this->addr));
	
	//ai_family参数指定调用者期待返回的套接口地址结构的类型
	//它的值包括三种：AF_INET，AF_INET6和AF_UNSPEC。如果指定AF_INET，
	//那么函数就不能返回任何IPV6相关的地址信息；如果仅指定了AF_INET6，
	//则就不能返回任何IPV4地址信息。AF_UNSPEC则意味着函数返回的是适用于指定
	//主机名和服务名且适合任何协议族的地址。如果某个主机既有AAAA记录(IPV6)地址，
	//同时又有A记录(IPV4)地址，那么AAAA记录将作为sockaddr_in6结构返回，
	//而A记录则作为sockaddr_in结构返回
	this->addr.sin_family = AF_INET;//AF_UNSPEC ;

	::inet_pton(AF_INET, this->ip.c_str(), &(this->addr.sin_addr));

	this->addr.sin_port = htons(this->port);

	net_util_t::set_reuse_addr(this->fd);

	int loop = 1;
	if (-1 == ::setsockopt(this->fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&loop, sizeof(loop))){
		ALERT_LOG("failed to set ip_multicast_loop [err_code:%d, err:%s]", errno, strerror(errno));
		return FAIL;
	}

	// set default interface for outgoing multicasts
	in_addr_t ipaddr;

	::inet_pton(AF_INET, this->out_if.c_str(), &ipaddr);
	if (::setsockopt(this->fd, IPPROTO_IP, IP_MULTICAST_IF, (char*)&ipaddr, sizeof ipaddr) == -1) {
		ALERT_LOG("failed to set outgoing interface [err_code:%d, err:%s, ip:%s]",
			errno, strerror(errno), this->out_if.c_str());
		return FAIL;
	}

	if (::bind(this->fd, (struct sockaddr*)&this->addr, sizeof(this->addr)) == -1) {
		ALERT_LOG("failed to bind mcast_fd [err_code:%d, err:%s]", errno, strerror(errno));
		return FAIL;
	}

	return this->mcast_join();
}

int mcast_t::mcast_join()
{
	struct group_req req;

	req.gr_interface = if_nametoindex(this->in_if.c_str());
	if (req.gr_interface == 0) {
		errno = ENXIO; /* i/f name not found */
		ALERT_LOG("failed [err_code:%d, err:%s, mcast_in:%s]", 
			errno, strerror(errno), this->in_if.c_str());
		return FAIL;
	}

	memcpy(&req.gr_group, &this->addr, sizeof(this->addr));
	if (-1 == ::setsockopt(this->fd, net_util_t::family_to_level(this->addr.sin_family), MCAST_JOIN_GROUP, (char*)&req, sizeof(req))) {
		ERROR_LOG("failed to join mcast grp [err_code:%d, err:%s]", errno, strerror(errno));
		return FAIL;
	}
	return SUCC;
}

mcast_t::mcast_t()
{
	this->port = 0;
}
}//end namespace xr