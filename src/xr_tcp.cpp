#include "xr_tcp.h"
#include "xr_file.h"
#include "xr_log.h"

namespace xr
{
int tcp_t::send(const void *data, int len)
{
	int send_bytes = 0;
	for (int cur_len = 0; send_bytes < len; send_bytes += cur_len)
	{
		//MSG_NOSIGNAL: linux man send 查看
		cur_len = (int)HANDLE_EINTR(::send(this->fd, (char *)data + send_bytes, len - send_bytes, MSG_NOSIGNAL));

		if (unlikely(-1 == cur_len))
		{
			if (EAGAIN == errno || EWOULDBLOCK == errno)
			{
				WARNI_LOG("send err [err_code:%u, err_msg:%s, fd:%d, len:%d, send_len:%d, ip:%s, port:%u]",
						  errno, ::strerror(errno), this->fd, len, send_bytes, xr::net_util_t::ip2str(this->ip), this->port);
				break;
			}
			else
			{
				WARNI_LOG("send err [err_code:%u, err_msg:%s, fd:%d, len:%d, send_len:%d, , ip:%s, port:%u]",
						  errno, ::strerror(errno), this->fd, len, send_bytes, xr::net_util_t::ip2str(this->ip), this->port);
				return FAIL;
			}
		}
	}
	return send_bytes;
}

int tcp_t::recv(void *data, int len)
{
	int recv_bytes = 0;

	for (int cur_len = 0; recv_bytes < len; recv_bytes += cur_len)
	{
		cur_len = (int)HANDLE_EINTR(::recv(this->fd, (char *)data + recv_bytes, len - recv_bytes, 0));
		if (likely((cur_len + recv_bytes) <= len))
		{
			return (cur_len + recv_bytes);
		}
		else if (cur_len == 0)
		{
			// connection closed by client
			return 0;
		}
		else if (-1 == cur_len)
		{
			if (EAGAIN == errno || EWOULDBLOCK == errno)
			{
				WARNI_LOG("recv err [err_code:%u, err_msg:%s, fd:%d, recv_len:%d]",
						  errno, ::strerror(errno), this->fd, recv_bytes);
				break;
			}
			else
			{
				WARNI_LOG("recv err [err_code:%u, err_msg:%s, fd:%d, recv_len:%d]",
						  errno, ::strerror(errno), this->fd, recv_bytes);
				return FAIL;
			}
		}
	}

	return recv_bytes;
}

int tcp_t::connect(const char *ip, uint16_t port,
				   int timeout, bool block, uint32_t send_buf_len, uint32_t rev_buf_len)
{
	struct sockaddr_in peer;
	::memset(&peer, 0, sizeof(peer));
	peer.sin_family = AF_INET;
	peer.sin_port = htons(port);
	if (::inet_pton(AF_INET, ip, &peer.sin_addr) <= 0)
	{
		ALERT_LOG("inet_pton err [err_code:%u, err_msg:%s, ip:%s, port:%u]",
				  errno, ::strerror(errno), ip, port);
		return INVALID_FD;
	}

	int fd = ::socket(PF_INET, SOCK_STREAM, 0);
	if (INVALID_FD == fd)
	{
		ALERT_LOG("create socket err [err_code:%u, err_msg:%s, ip:%s, port:%u]",
				  errno, ::strerror(errno), ip, port);
		return INVALID_FD;
	}

	//设置发送与接收缓存大小
	if (0 != send_buf_len)
	{
		net_util_t::set_sendbuf(fd, send_buf_len);
	}
	if (0 != rev_buf_len)
	{
		net_util_t::set_recvbuf(fd, rev_buf_len);
	}

	//------------------------
	// Works under Linux, although **UNDOCUMENTED**!!
	// 设置超时无用.要用select判断. 见unix网络编程connect
	if (timeout > 0)
	{
		net_util_t::set_sock_send_timeo(fd, timeout * 1000);
	}

	//connect 如果返回EINTR 不可再次调用，否则会返回一个错误(见：UNIX网络编程第三版，卷1，108页。)
	int ret = ::connect(fd, (struct sockaddr *)&peer, sizeof(peer));
	if (ret < 0)
	{
		ALERT_LOG("connect err [err_code:%u, err_msg:%s, ip:%s, port:%u]",
				  errno, ::strerror(errno), ip, port);
		file_t::close_fd(fd);
		return INVALID_FD;
	}

	if (timeout > 0)
	{
		net_util_t::set_sock_send_timeo(fd, 0);
	}

	file_t::set_io_block(fd, block);

	return fd;
}

tcp_peer_t::tcp_peer_t()
{
	this->init();
}

void tcp_peer_t::close()
{
	file_t::close_fd(this->fd);
	this->init();
}

void tcp_peer_t::init()
{
	this->fd = INVALID_FD;
	this->fd_type = FD_TYPE::UNUSED;
	this->port = 0;
	this->ip = 0;
	this->recv_buf.clear();
	this->send_buf.clear();
	this->data = NULL;
}

tcp_peer_t::~tcp_peer_t()
{
	this->close();
}

void tcp_peer_t::update(int fd, FD_TYPE fd_type, const char *ip, uint16_t port)
{
	this->init();
	this->fd = fd;
	this->fd_type = fd_type;
	if (NULL != ip)
	{
		this->ip = net_util_t::ip2int(ip);
		this->port = port;
	}
}
} //end namespace xr