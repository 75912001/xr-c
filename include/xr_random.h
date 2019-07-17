//随机数	
//linux: The largest number rand will return (same as INT_MAX)

#pragma once

#include "xr_util.h"

namespace xr{

	struct random_t{
		//产生给定范围内的随机数。如果需要更加随机，可以在程序起来的时候调用一次srand()	
		//min 随机出来的最小数
		//max 随机出来的最大数
		//return min和max之间的随机数，包括min和max//return [min,max]
		//linux: The largest number rand will return (same as INT_MAX)
		static inline int random(int min, int max){
			return (::rand()%(max-min+1))+min;
		}
		static inline uint32_t random_u32(uint32_t min, uint32_t max){
			return (::rand()%(max-min+1))+min;
		}

		static inline uint64_t random_u64(uint64_t min, uint64_t max){
			return (::rand()%(max-min+1))+min;
		}
		static inline int64_t random_64(int64_t min, int64_t max){
			return (::rand()%(max-min+1))+min;
		}
		//产生给定范围内的随机数。精度小数点后6位	
		static inline float random_f(float min = 0.000000f, float max = 1.000000f){
			static const float EPSILON = 1000000.0f;
			float r = (float)((float)(random_t::random_64((int64_t)(min * EPSILON), (int64_t)(max * EPSILON))) / EPSILON);
			if (r < min){
				return min;
			}
			if (r > max){
				return max;
			}
			return r;
		}
	};
}//end namespace xr