#include "xr_tcp_server.h"

namespace xr{
int tcp_srv_t::accept( struct sockaddr_in& peer, uint32_t sendbuf, uint32_t recvbuf, bool block  )
{
	socklen_t peer_size = sizeof(peer);

	int newfd = 0;
	do {
		newfd = HANDLE_EINTR(::accept(this->listen_fd, (struct sockaddr*)&peer, &peer_size));
	} while(unlikely(newfd < 0 
		&& (EWOULDBLOCK == errno || ECONNABORTED == errno || EPROTO == errno)));
	
	if (unlikely(newfd < 0)){
		return FAIL;
	}

	//设置发送与接收缓存大小
	if (0 != sendbuf){
		net_util_t::set_sendbuf(newfd, sendbuf);
	}
	if (0 != recvbuf){
		net_util_t::set_recvbuf(newfd, recvbuf);
	}

	file_t::set_io_block(newfd, block);

	return newfd;
}

int tcp_srv_t::bind( const char* ip, uint16_t port )
{
	sockaddr_in sa_in;
	::memset(&sa_in, 0, sizeof(sa_in));

	sa_in.sin_family = PF_INET;
	sa_in.sin_port = htons(port);
	if (NULL != ip){
		::inet_pton(PF_INET, ip, &sa_in.sin_addr);
	}else{
		sa_in.sin_addr.s_addr = htonl(INADDR_ANY);
	}

	return ::bind(this->listen_fd, (sockaddr*)&sa_in,sizeof(sa_in));
}

tcp_srv_t::tcp_srv_t()
{
	this->tcp_peer = NULL;
	this->listen_fd = INVALID_FD;
	this->cli_fd_value_max = 0;
}

tcp_srv_t::~tcp_srv_t()
{
	if (NULL != this->tcp_peer){
		for (int i = 0; i < this->cli_fd_value_max; i++) {
			tcp_peer_t& peer = this->tcp_peer[i];
			if (FD_TYPE_UNUSED == peer.fd_type){
				continue;
			}
			peer.close();
		}
		SAFE_DELETE_ARR(this->tcp_peer);
	}
	file_t::close_fd(this->listen_fd);
}

on_tcp_srv_t::on_tcp_srv_t()
{
	this->on_events = NULL;
	this->on_cli_pkg = NULL;
	this->on_srv_pkg = NULL;
	this->on_cli_conn = NULL;
	this->on_cli_conn_closed = NULL;
	this->on_svr_conn_closed = NULL;
	this->on_get_pkg_len = NULL;
	this->on_mcast_pkg = NULL;
	this->on_addr_mcast_pkg = NULL;
	this->on_udp_pkg = NULL;

	this->on_init = NULL;
	this->on_fini = NULL;
}
}//end namespace xr