//用于记录日志，一共分9种日志等级。
//注意,每条日志不能超过8000字节。
//init
//update_time...

//旧版本
//只在linux生效 使用线程
// 目前使用线程的话效率比不使用还低,应该在于NEW 和 delete ,加锁的过程.
// 不使用线程 使用 99秒 打印10004990行日志 101060行/秒(14.49W行/秒, 4G内存)
// 使用线程  使用 151秒 打印10004990行日志  66258行/秒

//当前版本
//  centos7 3.10.0-957.el7.x86_64
//  处理器名称：	Intel Core i7
//  处理器速度：	2.2 GHz
//  处理器数目：	4
//  内存：	1 GB
//  硬盘: 固态

//gcc -O2
//直接write写入 
//1000w写(478888900 = 478.88MB)14696504微妙=14.69秒 速度:68.4W行/s
//线程 写入(写入缓冲中的时间)缓冲大小4194304
//1000w写(478888900 = 478.88MB)5005706微妙=5.01秒 速度:199.6W行/s
//线程+semaphore
//1000w写(478888900 = 478.88MB)5107390微妙=5.11秒 速度:195.7W行/s

#pragma once

//使用线程记录日志
#define XR_USE_LOG_THREAD

#ifdef XR_USE_LOG_THREAD
	#include "xr_ring_buf.h"
	#include "xr_semaphore.h"
	#include "xr_thread.h"
#else
	#include "xr_util.h"
#endif

namespace xr{
	class log_thread_t
#ifdef XR_USE_LOG_THREAD
	: public thread_t{
	public:
		ring_buf_t ring_buf;
		semaphore_t semaphore;
		virtual void do_work_fn(void* data);
#else
	{
#endif
	};

	//日志等级
	enum E_LOG_LEVEL {
		//system is unusable
		E_LOG_LEVEL_EMERG = 0,
		//action must be taken immediately
		E_LOG_LEVEL_ALERT = 1,//黄色
		//critical conditions
		E_LOG_LEVEL_CRITI = 2,//深蓝
		//error conditions
		E_LOG_LEVEL_ERROR = 3,
		//warning conditions
		E_LOG_LEVEL_WARNI = 4,
		//normal but significant condition
		E_LOG_LEVEL_NOTIC = 5,
		//informational
		E_LOG_LEVEL_INFOR = 6,
		E_LOG_LEVEL_DEBUG = 7,
		E_LOG_LEVEL_TRACE = 8,
		E_LOG_LEVEL_MAX = 9,
	};

	//日志输出位置(屏幕/文件)
	enum E_LOG_DEST {//使用BIT位来写入数据.
		//仅输出到屏幕
		E_LOG_DEST_TERMINAL	= 1,
		//仅输出到文件
		E_LOG_DEST_FILE		= 2,
		//既输出到屏幕，也输出到文件
		E_LOG_DEST_BOTH		= 3
	};
	//日志的文件描述符
	struct log_fds_t {
		int		seq;//顺序号
		int		fd;//文件FD
		int		day;//一年中的天数20190626
		log_fds_t();
		virtual ~log_fds_t();
		void init();
	};

	class log_t{
	public:
		log_fds_t fds;
		log_thread_t thread;
	public:
		log_t();
		//初始化日志记录功能.按时间周期创建新的日志文件
		//dir:日志保存目录.如果填NULL,则在屏幕中输出日志
		//lvl:日志输出等级.如果设置为E_LOG_LEVEL_DEBUG,则E_LOG_LEVEL_DEBUG以上等级的日志都不输出
		//pre_name:日志文件名前缀
		//logtime:每个日志文件保存logtime分钟的日志,最大不能超过1440分钟
		//假设logtime为15,则每小时产生4个日志文件,每个文件保存15分钟日志
		//return:成功返回0,失败返回-1
		int init(const char* dir, xr::E_LOG_LEVEL lvl, const char* pre_name, uint32_t logtime);
		//由外部更新的时间(避免内部重复调用系统函数获取时间)
		inline void update_time(time_t log_sec, struct tm log_tm){
			this->log_sec = log_sec;
			this->log_tm = log_tm;
		}

		#ifdef __GNUC__
			#define LOG_CHECK_FMT(a,b) __attribute__((format(printf, a, b)))
		#else
			#define LOG_CHECK_FMT(a,b)
		#endif

		void boot(int ok, int space, const char* fmt, ...) LOG_CHECK_FMT(4, 5);
		void write(int lvl, const char* fmt, ...) LOG_CHECK_FMT(3, 4);

		//切换打开要写入文件的FD
		//returns:-1:ERROR,>0:返回打开的文件的FD
		int shift_fd();
	private:
		//由外部调用设置的时间(内部使用)
		time_t log_sec;
		struct tm log_tm;

		E_LOG_LEVEL level;
		E_LOG_DEST log_dest;
		std::string dir_name;//日志目录
		std::string file_pre_name;//文件名前缀
		uint32_t logtime_interval_sec;// 每个日志文件记录日志的总时间（秒）
		/**
		 * @brief	生成日志文件路径
		 * @param	char * file_name	产生的日志文件路径
		 * @param	const struct tm & t_m	时间
		 */
		void gen_log_file_path(char* file_name);

		//************************************
		// Brief:     获取日志序号
		// Returns:   int	日志序号
		//************************************
		int gen_log_seq();
		int open_fd();
	};
	extern log_t* g_log;
}//end namespace xr

#define DETAIL(level, fmt, args...) \
	xr::g_log->write(level, "[%s][%d]: " fmt "\n", __FILE__, __LINE__, ##args)
#define EMERG_LOG(fmt, args...) \
	DETAIL(xr::E_LOG_LEVEL_EMERG, fmt, ##args)
#define ALERT_LOG(fmt, args...) \
	DETAIL(xr::E_LOG_LEVEL_ALERT, fmt, ##args)
#define CRITI_LOG(fmt, args...) \
	DETAIL(xr::E_LOG_LEVEL_CRITI, fmt, ##args)
#define ERROR_LOG(fmt, args...) \
	DETAIL(xr::E_LOG_LEVEL_ERROR, fmt, ##args)
#define WARNI_LOG(fmt, args...) \
	DETAIL(xr::E_LOG_LEVEL_WARNI, fmt, ##args)
#define NOTIC_LOG(fmt, args...) \
	DETAIL(xr::E_LOG_LEVEL_NOTIC, fmt, ##args)
#define INFOR_LOG(fmt, args...) \
	DETAIL(xr::E_LOG_LEVEL_INFOR, fmt, ##args)
#define DEBUG_LOG(fmt, args...) \
	DETAIL(xr::E_LOG_LEVEL_DEBUG, fmt, ##args)
#define TRACE_LOG(fmt, args...) \
	DETAIL(xr::E_LOG_LEVEL_TRACE, fmt, ##args)

/**
 * @def BOOT_LOG
 * @brief 输出程序启动日志到屏幕
 *        用法示例：BOOT_LOG(-1, "dlopen error, %s", error);
 */

#define BOOT_LOG(__state__, fmt, args...) \
	do { \
		xr::g_log->boot(__state__, 0, fmt, ##args); \
	} while (0)
