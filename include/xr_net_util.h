//网络杂项

#pragma once

#include "xr_include.h"

namespace xr{
	enum E_FD_TYPE{
		FD_TYPE_UNUSED = 0,
		FD_TYPE_LISTEN = 1,
		FD_TYPE_PIPE = 2,
		FD_TYPE_CLI = 3,
		FD_TYPE_MCAST = 4,
		FD_TYPE_ADDR_MCAST = 5,
		FD_TYPE_UDP = 6,
		FD_TYPE_CONNECT = 7,
		FD_TYPE_SVR = 8,
	};

	struct net_util_t{
		//用于通过网卡接口（eth0/eth1/lo...）获取对应的IP地址。支持IPv4和IPv6。
		//nif 网卡接口.eth0/eth1/lo...
		//af 网络地址类型。AF_INET或者AF_INET6。
		//ipaddr 用于返回nif和af对应的IP地址。
		//return: 0:succ:0.-1:fail
		static int get_ip_addr(const char* nif, int af, std::string& ipaddr);
		
		//set a timeout on sending data. If you want to disable timeout, just simply call this function again with millisec set to be 0
		//return:0:success.-1:error and errno is set appropriately
		static inline int set_sock_send_timeo(int sockfd, int milliseconds){
			return net_util_t::set_sock_timeo(sockfd, SO_SNDTIMEO, milliseconds);
		}
		//set a timeout on receiving data. If you want to disable timeout, just simply call this function again with millisec set to be 0
		//return:0:success.-1:error and errno is set appropriately
		static inline int set_sock_recv_timeo(int sockfd, int milliseconds){
			return net_util_t::set_sock_timeo(sockfd, SO_RCVTIMEO, milliseconds);
		}
		//set the given fd recv buffer
		//returns:0:success.-1:error
		static inline int set_recvbuf(int sockfd, uint32_t len){
			return ::setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char*)&len, sizeof(len));
		}
		static inline int set_sendbuf(int sockfd, uint32_t len){
			return ::setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char*)&len, sizeof(len));
		}
		static inline uint32_t ip2int(const char* ip){
			return inet_addr(ip);
		}
		static inline char* ip2str(uint32_t ip){
			struct in_addr a;
			a.s_addr = ip;
			return inet_ntoa(a);
		}

		//translate the given address family to its corresponding level
		static int family_to_level(int family);
		//set the given fd SO_REUSEADDR
		//returns:0:success.-1:error
		static int set_reuse_addr(int fd){
			const int flag = 1;
			return ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag));
		}
	private:
		static inline int set_sock_timeo(int sockfd, int optname, int milliseconds){
			struct timeval tv;

			tv.tv_sec  = milliseconds / 1000;
			tv.tv_usec = (milliseconds % 1000) * 1000;

			return ::setsockopt(sockfd, SOL_SOCKET, optname, (char*)&tv, sizeof(tv));
		}
	};
}//end namespace xr
