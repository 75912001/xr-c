//定时器函数，有秒级和微妙级两种精度的接口.用于设定某一时刻调用某个函数	

#pragma once

#include "xr_include.h"
#include "xr_list.h"

// 如果回调函数返回0，则表示要删除该定时器，反之，则不删除	
typedef int (*ON_TIMER_FUN)(void* owner, void* data);

namespace xr{
	struct timer_t{
		struct timeval now;
		struct tm now_tm;
		//获取当前秒数
		uint32_t now_sec(){
			return (uint32_t)this->now.tv_sec;
		}
		//获取当前微秒数
		uint32_t now_usec(){
			return (uint32_t)this->now.tv_usec;
		}
		//扫描定时器列表,调用到期了的定时器的回调函数,ON_TIMER_FUN		
		//注意:调用前一定要调用过renew_time()
		void scan();
		//秒级定时器
		struct sec_t{
			//联系定时器转轮	
			list_head	entry;
			//联系定时器的拥有者	
			list_head	sprite_list;
			//定时器的触发时刻,亦即调用回调函数的时间	
			::time_t		expire;
			void*			owner;
			void*			data;
			ON_TIMER_FUN	function;
			void init();
		};
		//微秒级定时器	
		struct micro_t{
			list_head	entry;
			//定时器的触发时刻,亦即调用回调函数的时间	
			struct timeval	tv;
			void*			owner;
			void*			data;
			ON_TIMER_FUN	function;
			void init();
		};

		timer_t();
		virtual ~timer_t();
		//添加/替换一个秒级定时器，该定时器的到期时间是expire，到期时回调的函数是func	
		//return:指向新添加/替换的秒级定时器的指针	
		//head 链头，新创建的定时器会被插入到该链表中	
		//func 定时器到期时调用的回调函数	
		//owner 传递给回调函数的第一个参数	
		//data 传递给回调函数的第二个参数	
		//expire 定时器到期时间（从Epoch开始的秒数）	
		//注意:绝对不能在定时器的回调函数中修改该定时器的到期时间!	
		sec_t* add_sec(list_head_t* head, ON_TIMER_FUN func,
			void* owner, void* data, ::time_t expire);
		//删除指定的秒级定时器	
		void rm_sec(sec_t* t);
		//删除链表head中所有的秒级定时器	
		//head 定时器链表的链头	
		void rm_all_sec(list_head_t* head);

		//添加一个微秒级定时器	
		//return:指向新添加的微秒级定时器的指针	
		//func 回调的函数	
		//tv 定时器到期时间	
		//owner 传递给回调函数的第一个参数	
		//data 传递给回调函数的第二个参数	
		micro_t* add_micro(ON_TIMER_FUN func, 
			const timeval* tv, void* owner, void* data);
		void rm_micro(micro_t *t);
		//删除传递给回调函数的第一个参数==owner的所有微秒级定时器
		void rm_all_micro(void* owner);
		//更新当前时间	
		inline void renew_time(){
			::gettimeofday(&this->now, 0);
 			::time_t second = this->now.tv_sec;
 			::localtime_r(&second, &this->now_tm);
		}
	private:
		enum {
			E_TIMER_VEC_SIZE = 5,
		};
		struct tvec_root_s {
			list_head head;
			::time_t     expire;
			//最小到期时间	
			::time_t  min_expiring_time;
			tvec_root_s();
		};
		::time_t last_handle_sec;
		tvec_root_s  vec[E_TIMER_VEC_SIZE];
		list_head    micro_timer;//毫秒是只有一个链表.每次全部检索	


		inline int find_min_idx(::time_t diff, int max_idx){
			while (max_idx && (this->vec[max_idx - 1].expire >= diff)) {
				--max_idx;
			}
			return max_idx;
		}
		//更新秒级定时器	
		void renew_sec_timer_list(const int idx);
		//设置定时轮中一个转轮中到期最小时间	
		//exptm 到期时间	
		//idx 定时轮中的序号	
		inline void set_min_exptm(::time_t exptm, int idx){
			if ((exptm < this->vec[idx].min_expiring_time) 
				|| (0 == this->vec[idx].min_expiring_time)) {
				this->vec[idx].min_expiring_time = exptm;
			}
		}
		//根据到期时间找时间轮的序号	
		//return:时间轮序号	
		inline int find_root_idx(::time_t expire_time){
			int idx = 0;
			::time_t diff = expire_time - this->now.tv_sec;
			for (; idx != (E_TIMER_VEC_SIZE - 1); ++idx) {
				if (diff <= this->vec[idx].expire){
					break;
				}
			}
			return idx;
		}
		//增加一个定时	
		inline void add_sec(sec_t *t){
			int idx = this->find_root_idx(t->expire);
			list_add_tail(&t->entry, &this->vec[idx].head);
			this->set_min_exptm(t->expire, idx);
		}
		//扫描毫秒定时器链表(逐个)	
		void scan_micro();
		//扫描秒级定时器离链表(转轮序号0)	
		void scan_sec();
	};
	extern timer_t* g_timer;

	//使用定时器的事件	
	class timer_event_t{
	public:
		list_head_t timer_list;
		timer_event_t(timer_t* timer);
		virtual ~timer_event_t();

		//创建一个新的秒级定时器,传递给回调函数的第一个参数是this,第二个参数是data	
		//指向新创建的秒级定时器的指针	
		//func 回调函数	
		//data 第二个参数	
		//expire 该定时器的到期时间是expire（从Epoch开始的秒数）	
		timer_t::sec_t* add_sec(ON_TIMER_FUN func, void* data, ::time_t expire);

		//删除秒级定时器timer_sec
		void remove_sec(timer_t::sec_t* timer_sec);

		//创建一个新的微妙级定时器,传递给回调函数的第一个参数是this,第二个参数是data	
		//return:指向新创建的微妙级定时器的指针	
		//func 回调函数	
		//data 第二个参数	
		//expire 该定时器的到期时间是expire	
		timer_t::micro_t* add_micro(ON_TIMER_FUN func, void* data, timeval* expire);
	
		//删除微秒级定时器timer_micro	
		void remove_micro(timer_t::micro_t* timer_micro);
		//删除所有秒级定时器 && 删除所有微秒级定时器	
		void remove_all();
	private:
		timer_t* timer;
	};
}//end namespace xr