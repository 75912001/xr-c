//tcp server

#pragma once

#include "xr_tcp.h"

namespace xr
{
//服务器回调函数
struct on_tcp_srv_t
{
	//the following interfaces are called only by the child process
	//called each time before processing packages from clients
	//calling interval of this interface is no much longer than 100ms at maximum
	typedef void (*ON_EVENTS)();
	ON_EVENTS on_events;
	//called each time when a client connection
	typedef void (*ON_CLI_CONN)(xr::tcp_peer_t *tcp_peer);
	ON_CLI_CONN on_cli_conn;
	//called to process packages from clients. called once for each package
	//return xr::ERR_SYS::DISCONNECT_PEER if you want to close the connection,`on_cli_conn_closed` will be called too
	typedef int (*ON_CLI_PKG)(xr::tcp_peer_t *tcp_peer, const void *data, uint32_t len);
	ON_CLI_PKG on_cli_pkg;
	//called to process packages from servers that the child connects to. called once for each package.
	//return xr::ERR_SYS::DISCONNECT_PEER if you want to close the connection,`on_svr_conn_closed` will be called too
	typedef void (*ON_SRV_PKG)(xr::tcp_peer_t *tcp_peer, const void *data, uint32_t len);
	ON_SRV_PKG on_srv_pkg;
	//called each time when close a connection
	typedef void (*ON_CLI_CONN_CLOSED)(int fd);
	ON_CLI_CONN_CLOSED on_cli_conn_closed;
	//called each time when close a connection
	typedef void (*ON_SVR_CONN_CLOSED)(int fd);
	ON_SVR_CONN_CLOSED on_svr_conn_closed;
	//you must return 0 if you cannot yet determine the length of the incoming package,
	//return -1 if you find that the incoming package is invalid and will close the connection,
	//otherwise, return the length of the incoming package
	typedef int (*ON_GET_PKG_LEN)(xr::tcp_peer_t *tcp_peer, const void *data, uint32_t len);
	ON_GET_PKG_LEN on_get_pkg_len;
	//called to process multicast packages from the specified `mcast_ip` and `mcast_port`. called once for each package
	typedef void (*ON_MCAST_PKG)(const void *data, uint32_t len);
	ON_MCAST_PKG on_mcast_pkg;
	//called to process multicast packages from the specified `addr_mcast_ip` and `addr_mcast_port`. called once for each package
	//flag:1:可用.0:不可用
	typedef void (*ON_ADDR_MCAST_PKG)(uint32_t id, const char *name, const char *ip, uint16_t port, const char *data, uint32_t flag);
	ON_ADDR_MCAST_PKG on_addr_mcast_pkg;
	//未使用todo
	typedef void (*ON_UDP_PKG)(int fd, const void *data, uint32_t len, struct sockaddr_in *from, socklen_t fromlen);
	ON_UDP_PKG on_udp_pkg;
	//the following interfaces are called both by the parent and child process
	//called only once at server startup by both the parent and child process
	//initialize your service program (allocate memory, create objects, etc)
	//return:0:success.-1:file
	typedef int (*ON_INIT)();
	ON_INIT on_init;
	//called only once at server stop by both the parent and child process
	//finalize your service program (release memory, destroy objects, etc)
	//return:0: if you have finished finalizing the service.-1:otherwise.
	typedef int (*ON_FINI)();
	ON_FINI on_fini;

	on_tcp_srv_t();
};

//tcp服务基类
class tcp_srv_t : public tcp_t
{
public:
	tcp_peer_t *tcp_peer; //连接用户的信息
	int cli_fd_value_max; //连接上的FD中的最大值
	int listen_fd;		  //监听FD	public:
	tcp_srv_t();
	virtual ~tcp_srv_t();
	virtual int create() = 0;
	virtual int listen(const char *ip, uint16_t port, uint32_t listen_num, int bufsize) = 0;
	virtual int run() = 0;
	virtual tcp_peer_t *add_connect(int fd, FD_TYPE fd_type, const char *ip, uint16_t port) = 0;

	//accept a tcp connection
	//peer:used to return the protocol address of the connected peer process
	//block:true:accepted fd will be set blocking.false:fd will be set nonblocking
	//return:the accpected fd on success.-1:fail
	int accept(struct sockaddr_in &peer, uint32_t sendbuf, uint32_t recvbuf, bool block = false);

protected:
	virtual int bind(const char *ip, uint16_t port);
};
} //end namespace xr