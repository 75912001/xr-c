#include "xr_net_util.h"
#include "xr_util.h"

namespace xr
{
int net_util_t::get_ip_addr(const char *nif, int af, std::string &ipaddr)
{
	int ret_code = FAIL;
	char ip[INET6_ADDRSTRLEN] = {0};
	assert((af == AF_INET) || (af == AF_INET6));

	// get a list of network interfaces
	struct ifaddrs *ifaddr;
	if (::getifaddrs(&ifaddr) < 0)
	{
		return FAIL;
	}

	// walk through linked list
	int err = EADDRNOTAVAIL;

	struct ifaddrs *ifa; // maintaining head pointer so we can free list later
	for (ifa = ifaddr; ifa != 0; ifa = ifa->ifa_next)
	{
		if ((ifa->ifa_addr == 0) || (ifa->ifa_addr->sa_family != af) || ::strcmp(ifa->ifa_name, nif))
		{
			continue;
		}
		// convert binary form ip address to numeric string form
		ret_code = getnameinfo(ifa->ifa_addr,
							   (af == AF_INET) ? (socklen_t)sizeof(struct sockaddr_in) : (socklen_t)sizeof(struct sockaddr_in6),
							   (char *)ip, INET6_ADDRSTRLEN, 0, 0, NI_NUMERICHOST);
		if (SUCC != ret_code)
		{
			if (EAI_SYSTEM == ret_code)
			{
				err = errno;
			}
			ret_code = FAIL;
		}
		break;
	}

	::freeifaddrs(ifaddr);
	errno = err;
	ipaddr = ip;
	return ret_code;
}

int xr::net_util_t::family_to_level(int family)
{
	int level = -1;

	switch (family)
	{
	case AF_INET:
		level = IPPROTO_IP;
		break;
	case AF_INET6:
		level = IPPROTO_IPV6;
		break;
	default:
		break;
	}

	return level;
}

} //end namespace xr