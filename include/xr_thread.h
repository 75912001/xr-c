//线程

#pragma once

//<errno.h> 会把errno扩展成一个函数,访问errno变量的某个局限于线程的副本
//见unix网络编程第二版卷2,9页
//#define _POSIX_C_SOURCE	199506L
#define _REENTRANT

#include "xr_include.h"

namespace xr
{
class thread_t
{
public:
	thread_t();
	virtual ~thread_t();

	//pdata应用层调用的函数参数(需要应用层回收)
	int start(void *data = NULL);
	int stop();
	//开始后,线程先运行该函数(会反复运行,直到返回SUCC)
	virtual int do_work_begin();
	//true == is_sun ? 反复运行 : 运行结束
	virtual void do_work_fn(void *data) = 0;
	//最后运行该函数(会反复运行,直到返回SUCC)
	virtual int do_work_end();
	void *data;
	bool is_run;

private:
	pthread_t id;
};
} // namespace xr