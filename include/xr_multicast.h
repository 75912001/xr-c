//组播
#pragma once

#include "xr_udp.h"

namespace xr{
	class mcast_t : public udp_t{
	protected:
		std::string in_if;//recv
		std::string out_if;//send
		std::string ip;//239.X.X.X
		uint16_t port;//239.X.X.X
	public:
		mcast_t();
		//return 0:succ
		int create(const char* ip, uint16_t port,
			const char* in_if, const char* out_if);
	private:
		int mcast_join();
	};
	extern mcast_t* g_mcast;
}//end namespace xr
