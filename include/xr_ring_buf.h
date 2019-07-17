//环形缓冲
//单线程写,单线程读

#pragma once

#include "xr_util.h"

	//gcc -O0
	//25500w写读(每次写入1byte,读1byte)25930ms 速度:9834次/ms 983W/s

	//gcc -O2
	//25500w写读(每次写入1byte,读1byte)21430ms 速度:11899次/ms 1189W/s

	//gcc -O0
	//pthread_mutex_t
	//25500w写读(每次写入1byte,读1byte)64676ms 速度:3942次/ms 394W/s

	//  centos7 3.10.0-957.el7.x86_64
	//  处理器名称：	Intel Core i7
	//  处理器速度：	2.2 GHz
	//  处理器数目：	4
	//  内存：	1 GB

//////////////////////////////////////////////////////////////////////////
//内存屏障

namespace xr{
//取模运算
#define RING_BUF_MOD(__n__) ((__n__) & (this->size - 1))

	//template <typename T_ELEM>
	struct ring_buf_t{
		ring_buf_t(){
			this->init();
		}
		virtual ~ring_buf_t(){
			this->clear();
		}
		//len:2的幂
		//return:-1:失败
		int create(uint32_t len){
			if (!this->is_power_of_2(len)){
				return FAIL;
			}
			this->buffer = (unsigned char*)::realloc(this->buffer, len);
			if (NULL == this->buffer){
				return FAIL;
			}
			this->size = len;
			return 0;
		}
		//return: 返回放入的长度
		//放不下的丢掉
		inline uint32_t write(const void* data, uint32_t len){
			//可写入长度
			const uint32_t write_len = std::min(this->size - (this->in - this->out), len);

   			const uint32_t l = std::min(write_len, this->size - RING_BUF_MOD(this->in));
			::memcpy(this->buffer + RING_BUF_MOD(this->in), data, l);
			::memcpy(this->buffer, (unsigned char*)data + write_len, write_len -l);
			//smp_wmb就是一个写操作内存屏障，在它之前的写操作当必须在它之后的写操作前完成，
			//如果没有这个写操作屏障，由于cpu的乱序机制，可能把更新索引操作提前，那么就会造成缓冲区的混乱了。
			//smp_wmb();
			this->in += write_len;
			return write_len;
		}
		//return:返回取出的长度
		inline uint32_t read(void* data, uint32_t len){
			const uint32_t read_len = std::min(this->in - this->out, len);

    		const uint32_t l = std::min(read_len, this->size - RING_BUF_MOD(this->out));
    		::memcpy(data, this->buffer + RING_BUF_MOD(this->out), l);
    		::memcpy((unsigned char*)data + l, this->buffer, read_len - l);
			//smp_wmb(); 
			this->out += read_len;
			return read_len;
		}
	private:
		//用于存放数据的缓存
		unsigned char* buffer;
		//缓冲区空间的大小，在初化时，将它向上圆整成2的幂
		uint32_t size;
		//指向buffer中队头
		uint32_t in;
		//指向buffer中的队尾
		uint32_t out;
		inline void clear(){
			SAFE_FREE(this->buffer);
			this->init();
		}
		inline void init(){
			this->buffer = NULL;
			this->in = 0;
			this->out = 0;
			this->size = 0;
		}
		//判断一个数是否为2的次幂
		inline bool is_power_of_2(uint32_t n){
			return (n != 0 && ((n & (n - 1)) == 0));
 		}
	};
}//end namespace xr