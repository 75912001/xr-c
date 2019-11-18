//时间函数

#pragma once

#include "xr_include.h"

namespace xr
{
struct time_t
{
	static const uint32_t HOUR_SEC = 60 * 60;
	static const uint32_t DAY_SEC = HOUR_SEC * 24;
	static const uint32_t WEEK_SEC = DAY_SEC * 7;
	static const uint32_t TIME_SEC_MAX = INT_MAX;
	//returns the time as the number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC).
	static inline ::time_t now_second()
	{
		return ::time(NULL);
	}
	static inline struct tm cal_tm(::time_t second)
	{
		struct tm t;
		::localtime_r(&second, &t);
		return t;
	}
	//由年,月,日,小时,分钟,秒.计算时间秒数
	static inline ::time_t cal_second(int year, int mon = 1, int mday = 1, int hour = 0, int min = 0, int sec = 0)
	{
		struct tm t;
		t.tm_year = year - 1900;
		t.tm_mon = month - 1;
		t.tm_mday = day;
		t.tm_hour = hour;
		t.tm_min = minute;
		t.tm_sec = sec;
		return ::mktime(&t);
	}
	//由传入struct tm,计算出ymdhms
	static inline void cal_ymdhms(const struct tm *t, int &year,
								  int &mon, int &mday, int &hour, int &min, int &sec)
	{
		year = t->tm_year + 1900;
		mon = t->tm_mon + 1;
		mday = t->tm_mday;
		hour = t->tm_hour;
		min = t->tm_min;
		sec = t->tm_sec;
	}
	//获取当天日期
	//return 格式为"YYYYMMDD"
	static inline int cal_yyyymmdd(const struct tm *t)
	{
		return (t->tm_year + 1900) * 10000 + (t->tm_mon + 1) * 100 + t->tm_mday;
	}
	//获取当天年月
	//return 格式为"YYYYMM"
	static inline int cal_yyyymm(const struct tm *t)
	{
		return (t->tm_year + 1900) * 100 + t->tm_mon + 1;
	}
	//计算下一个时分秒的时间
	static inline ::time_t cal_next_second(::time_t second, int next_hour = 0, int next_min = 0, int next_sec = 0)
	{
		struct tm t = time_t::cal_tm(second);

		t.tm_hour = next_hour;
		t.tm_min = next_min;
		t.tm_sec = next_sec;

		::time_t gen_t;
		gen_t = ::mktime(&t);
		return gen_t < second ? gen_t + DAY_SEC : gen_t;
	}
	//由星期，小时，分钟, 秒计算下次时间秒数(当前时间)
	static inline ::time_t cal_next_second(::time_t second, int wday, int hour = 0, int min = 0, int sec = 0)
	{
		struct tm t = time_t::cal_tm(second);

		if (t.tm_wday == wday)
		{
			::time_t next_s = hour * HOUR_SEC + min * 60 + sec;
			::time_t s = t.tm_hour * HOUR_SEC + t.tm_min * 60 + t.tm_sec;
			if (s == next_s)
			{
				return second;
			}
			if (next_s > s)
			{
				return second + (next_s - s);
			}
			if (next_s < s)
			{
				t.tm_mday += 7 - (t.tm_wday - wday);
			}
		}
		else if (t.tm_wday < wday)
		{
			t.tm_mday += wday - t.tm_wday;
		}
		else
		{
			t.tm_mday += 7 - (t.tm_wday - wday);
		}

		t.tm_hour = hour;
		t.tm_min = min;
		t.tm_sec = sec;

		return ::mktime(&t);
	}

	//计算当日零点时间
	static inline ::time_t cal_day_start_time(struct tm t)
	{
		t.tm_hour = 0;
		t.tm_min = 0;
		t.tm_sec = 0;
		return ::mktime(&t);
	}

	//计算本小时开始时间
	static inline ::time_t cal_hour_start_time(::time_t second)
	{
		struct tm t = time_t::cal_tm(second);
		return second - t.tm_min * 60 - t.tm_sec;
	}

	//计算从0开始,第几天
	static inline uint32_t day_number(struct tm t)
	{
		::time_t sec = time_t::cal_day_start_time(t);
		return sec / DAY_SEC;
	}

	//计算下个月开始零点时间
	static inline ::time_t cal_next_month_start_time(struct tm *t)
	{
		int yyyymm = time_t::cal_yyyymm(t);
		int yyyy = (yyyymm / 100);
		int mm = (yyyymm % 100);
		if (12 == mm)
		{
			yyyy++;
			mm = 1;
		}
		else
		{
			mm++;
		}

		return time_t::cal_second(yyyy, mm);
	}

	//计算是从时间戳开始,是第几周(按照周一开始,周日结束计算)
	static inline uint32_t cal_week_num(::time_t second)
	{
		const time_t t1 = 0;
		struct tm t;
		::localtime_r(&t1, &t);

		int tm_wday = (0 == t.tm_wday ? 7 : t.tm_wday);
		int sec = t.tm_hour * HOUR_SEC + t.tm_min * 60 + t.tm_sec + (tm_wday - 1) * DAY_SEC;

		second += sec;
		return (uint32_t)(second / WEEK_SEC + 1);
	}

	//计算今天是周几(周一:1 ~ 周日:7)
	static inline uint32_t time_week_today(struct tm *t)
	{
		return 0 == t->tm_wday ? 7 : t->tm_wday;
	}
};
} //end namespace xr