//tcp

#pragma once

#include "xr_net.h"
#include "xr_memory.h"
#include "xr_net_util.h"

namespace xr{
	
	class tcp_t : public net_t
	{
	public:
		uint16_t port;
		uint32_t ip;
		tcp_t(){
			this->port = 0;
			this->ip = 0;
		}
		virtual int send(const void* data, int len);
		virtual int recv(void* data, int len);

		//create a tcp connection
		//timeout:abort the connecting attempt after timeout secs. If timeout is less than or equal to 0, 
		//then the connecting attempt will not be interrupted until error occurs
		//block:true and the connected fd will be set blocking, false and the fd will be set nonblocking.
		//send_buf_len:发送缓冲大小
		//rev_buf_len:接收缓冲大小
		//return:the connected fd
		static int connect( const char* ip, uint16_t port, int timeout,
			bool block, uint32_t send_buf_len = 0, uint32_t rev_buf_len = 0 );

	};

	class tcp_peer_t : public tcp_t
	{
	public:
		E_FD_TYPE fd_type;
		active_buf_t recv_buf;
		active_buf_t send_buf;
		void* data;//用户数据
		tcp_peer_t();
		virtual ~tcp_peer_t();
		//更新数据
		void update(int fd, E_FD_TYPE fd_type, const char* ip, uint16_t port);
		void close();
		void init();
	};
}//end namespace xr
