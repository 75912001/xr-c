#include "xr_log.h"
#include "xr_file.h"

namespace
{
const uint32_t LOG_BUF_SIZE = 81920;	//每条日志的最大字节数
const uint32_t RING_BUF_SIZE = 4194304; //环形缓冲大小(2的整数次幂)

const char *s_log_color[] = {"\e[1m\e[35m", "\e[1m\e[33m",
							 "\e[1m\e[34m", "\e[1m\e[31m", "\e[1m\e[32m", "\e[1m\e[36m",
							 "\e[1m\e[1m", "\e[1m\e[37m", "\e[1m\e[37m"};
const char *s_color_end = "\e[m";

static const char *log_names[] = {"emerg", "alert", "criti", "error", "warni", "notic", "infor", "debug", "trace"};
} //end of namespace

namespace xr
{
log_t *g_log = NULL;
int log_t::init(const char *dir, E_LOG_LEVEL lvl, const char *pre_name, uint32_t logtime)
{
	if (1440 < logtime)
	{
		::fprintf(stderr, "init log logtime too big=%u!!!\n", logtime);
		BOOT_LOG(FAIL, "log [dir:%s, per_name:%s, interval_time:%u]", dir, pre_name, logtime);
		return FAIL;
	}

	if (NULL == dir || (0 == ::strlen(dir)))
	{
		::fprintf(stderr, "init log dir is NULL!!!\n");
		BOOT_LOG(FAIL, "log [dir:%s, per_name:%s, interval_time:%u]", dir, pre_name, logtime);
		return FAIL;
	}

	if ((lvl < E_LOG_LEVEL_EMERG) || (lvl >= E_LOG_LEVEL_MAX))
	{
		::fprintf(stderr, "init log error, invalid log level=%d\n", lvl);
		BOOT_LOG(FAIL, "log [dir:%s, per_name:%s, interval_time:%u]", dir, pre_name, logtime);
		return FAIL;
	}

	//必须可写
	if (0 != ::access(dir, W_OK))
	{
		::fprintf(stderr, "access log dir %s error\n", dir);
		BOOT_LOG(FAIL, "log [dir:%s, per_name:%s, interval_time:%u]", dir, pre_name, logtime);
		return FAIL;
	}
#ifdef XR_USE_LOG_THREAD
	if (SUCC != this->thread.semaphore.init())
	{
		BOOT_LOG(FAIL, "log [dir:%s, per_name:%s, interval_time:%u]", dir, pre_name, logtime);
		return FAIL;
	}
	if (SUCC != this->thread.ring_buf.create(RING_BUF_SIZE))
	{
		BOOT_LOG(FAIL, "log [dir:%s, per_name:%s, interval_time:%u]", dir, pre_name, logtime);
		return FAIL;
	}
	if (SUCC != this->thread.start(this))
	{
		BOOT_LOG(FAIL, "log [dir:%s, per_name:%s, interval_time:%u]", dir, pre_name, logtime);
		return FAIL;
	}
#endif
	this->dir_name = dir;
	this->level = lvl;
	if (NULL == pre_name)
	{
		this->file_pre_name.clear();
	}
	else
	{
		this->file_pre_name = pre_name;
	}
	this->logtime_interval_sec = logtime * 60;

	this->fds.init();
	this->fds.seq = gen_log_seq();

	this->log_dest = E_LOG_DEST_FILE;
	return SUCC;
}

void log_t::boot(int ok, int space, const char *fmt, ...)
{
	int end, pos;
	va_list ap;

	static char log_buffer[LOG_BUF_SIZE];

	va_start(ap, fmt);
	end = ::vsprintf(log_buffer, fmt, ap);
	va_end(ap);

	static const int SCREEN_COLS = 80;
	pos = SCREEN_COLS - 10 - (end - space) % SCREEN_COLS;
	int i = 0;
	for (; i < pos; i++)
	{
		log_buffer[end + i] = ' ';
	}
	log_buffer[end + i] = '\0';

	static const char *BOOT_OK = "\e[1m\e[32m[ OK ]\e[m";
	static const char *BOOT_FAIL = "\e[1m\e[31m[ ERR ]\e[m";

	::strcat(log_buffer, SUCC == ok ? BOOT_OK : BOOT_FAIL);
	::printf("\r%s\n", log_buffer);

	if (SUCC != ok)
	{
		::exit(ok);
	}
}

int log_t::shift_fd()
{
	if (unlikely((this->fds.fd < 0) && (this->open_fd() < 0)))
	{
		return ERR;
	}

	if (0 != this->logtime_interval_sec)
	{
		if (likely(this->fds.seq == gen_log_seq()))
		{
			return SUCC;
		}
	}
	else
	{
		if (likely((this->fds.day == this->log_tm.tm_yday)))
		{
			return SUCC;
		}
	}

	file_t::close_fd(this->fds.fd);

	if (0 != this->logtime_interval_sec)
	{
		this->fds.seq = gen_log_seq();
	}

	int ret = open_fd();

	return ret;
}

int log_t::gen_log_seq()
{
	return (int)(0 == this->logtime_interval_sec ? 0 : (uint32_t)this->log_sec / this->logtime_interval_sec);
}

int log_t::open_fd()
{
	//O_APPEND 有该选项,write时是线程安全的.请看本页write函数
	int flag = O_WRONLY | O_CREAT | O_APPEND /* | O_LARGEFILE*/;

	char file_name[FILENAME_MAX];
	gen_log_file_path(file_name);

	this->fds.fd = ::open(file_name, flag, 0644);
	if (INVALID_FD != this->fds.fd)
	{
		if (this->fds.day != this->log_tm.tm_yday)
		{
			this->fds.day = this->log_tm.tm_yday;
		}
		file_t::fcntl_add_flag(this->fds.fd, FD_CLOEXEC);
	}

	return this->fds.fd;
}

void log_t::gen_log_file_path(char *file_name)
{
	if (0 != this->logtime_interval_sec)
	{
		time_t t = this->log_sec / this->logtime_interval_sec * this->logtime_interval_sec;
		struct tm tmp_tm;
		::localtime_r(&t, &tmp_tm);

		::sprintf(file_name, "%s/%s%04d%02d%02d%02d%02d.txt",
				  this->dir_name.c_str(), this->file_pre_name.c_str(),
				  tmp_tm.tm_year + 1900, tmp_tm.tm_mon + 1,
				  tmp_tm.tm_mday, tmp_tm.tm_hour, tmp_tm.tm_min);
	}
	else
	{
		::sprintf(file_name, "%s/%s%04d%02d%02d.txt",
				  this->dir_name.c_str(), this->file_pre_name.c_str(),
				  this->log_tm.tm_year + 1900, this->log_tm.tm_mon + 1, this->log_tm.tm_mday);
	}
}

log_t::log_t()
{
	this->level = E_LOG_LEVEL_INFOR;
	this->log_dest = E_LOG_DEST_TERMINAL;
	this->logtime_interval_sec = 0;
	{
		this->log_sec = ::time(NULL);
		::localtime_r(&this->log_sec, &this->log_tm);
	}
}

#ifdef XR_USE_LOG_THREAD
void log_t::write(int lvl, const char *fmt, ...)
{
	if (lvl > this->level)
	{
		return;
	}

	va_list ap;

	va_start(ap, fmt);

	if (unlikely(E_LOG_DEST_TERMINAL & this->log_dest))
	{
		va_list aq;
		va_copy(aq, ap);
		switch (lvl)
		{
		case E_LOG_LEVEL_EMERG:
		case E_LOG_LEVEL_ALERT:
		case E_LOG_LEVEL_CRITI:
		case E_LOG_LEVEL_ERROR:
			::fprintf(stderr, "%s%02d:%02d:%02d ", s_log_color[lvl],
					  this->log_tm.tm_hour, this->log_tm.tm_min, this->log_tm.tm_sec);
			::vfprintf(stderr, fmt, aq);
			::fprintf(stderr, "%s", s_color_end);
			break;
		default:
			::fprintf(stdout, "%s%02d:%02d:%02d ", s_log_color[lvl],
					  this->log_tm.tm_hour, this->log_tm.tm_min, this->log_tm.tm_sec);
			::vfprintf(stdout, fmt, aq);
			::fprintf(stdout, "%s", s_color_end);
			break;
		}
		va_end(aq);
	}

	if (unlikely(!(this->log_dest & E_LOG_DEST_FILE)))
	{
		va_end(ap);
		return;
	}

	static char log_buffer[LOG_BUF_SIZE];

	int pos = ::snprintf(log_buffer, sizeof(log_buffer), "[%02d:%02d:%02d][%s]",
						 this->log_tm.tm_hour, this->log_tm.tm_min, this->log_tm.tm_sec, log_names[lvl]);

	int end = ::vsnprintf(log_buffer + pos, sizeof(log_buffer) - pos, fmt, ap);
	va_end(ap);

	uint32_t len = end + pos;
	uint32_t write_len = 0;
	do
	{
		uint32_t n = this->thread.ring_buf.write(log_buffer + write_len, len - write_len);
#if RING_BUF_SIZE < LOG_BUF_SIZE
		if (0 < n)
		{
			//RING_BUF_SIZE < LOG_BUF_SIZE 时 信号量放在此处发送
			this->thread.semaphore.post();
		}
#endif
		write_len += n;
	} while (len != write_len);
#if LOG_BUF_SIZE <= RING_BUF_SIZE
	//LOG_BUF_SIZE <= RING_BUF_SIZE 时 信号量放在此处发送
	this->thread.semaphore.post();
#endif
}
#else
void log_t::write(int lvl, const char *fmt, ...)
{
	if (lvl > this->level)
	{
		return;
	}

	va_list ap;

	va_start(ap, fmt);

	if (unlikely(E_LOG_DEST_TERMINAL & this->log_dest))
	{
		va_list aq;
		va_copy(aq, ap);
		switch (lvl)
		{
		case E_LOG_LEVEL_EMERG:
		case E_LOG_LEVEL_ALERT:
		case E_LOG_LEVEL_CRITI:
		case E_LOG_LEVEL_ERROR:
			::fprintf(stderr, "%s%02d:%02d:%02d ", s_log_color[lvl],
					  this->log_tm.tm_hour, this->log_tm.tm_min, this->log_tm.tm_sec);
			::vfprintf(stderr, fmt, aq);
			::fprintf(stderr, "%s", s_color_end);
			break;
		default:
			::fprintf(stdout, "%s%02d:%02d:%02d ", s_log_color[lvl],
					  this->log_tm.tm_hour, this->log_tm.tm_min, this->log_tm.tm_sec);
			::vfprintf(stdout, fmt, aq);
			::fprintf(stdout, "%s", s_color_end);
			break;
		}
		va_end(aq);
	}

	if (unlikely(!(this->log_dest & E_LOG_DEST_FILE) || (shift_fd() < 0)))
	{
		va_end(ap);
		return;
	}

	static char log_buffer[LOG_BUF_SIZE];

	int pos = ::snprintf(log_buffer, sizeof(log_buffer), "[%02d:%02d:%02d][%s]",
						 this->log_tm.tm_hour, this->log_tm.tm_min, this->log_tm.tm_sec, log_names[lvl]);

	int end = ::vsnprintf(log_buffer + pos, sizeof(log_buffer) - pos, fmt, ap);
	va_end(ap);

	//判断返回值(会被信号中断等处理方式).. on linux :  man write
	//多线程写文件,这里不需要上锁!请 man 2 write
	// If the file  was open(2)ed  with  O_APPEND,
	// the file offset is first set to the end of the file before writing.
	// The adjustment of the file offset and the write operation are performed as an atomic step.
	HANDLE_EINTR(::write(this->fds.fd, log_buffer, end + pos));
}
#endif

void log_fds_t::init()
{
	this->seq = 0;
	this->fd = INVALID_FD;
	this->day = 0;
}

log_fds_t::log_fds_t()
{
	this->init();
}

log_fds_t::~log_fds_t()
{
	file_t::close_fd(this->fd);
}

#ifdef XR_USE_LOG_THREAD
void log_thread_t::do_work_fn(void *data)
{
	log_t *log = (log_t *)data;
	log->thread.semaphore.wait();
	if (unlikely(log->shift_fd() < 0))
	{
		return;
	}
	static char log_buffer[RING_BUF_SIZE];
	uint32_t n = this->ring_buf.read(log_buffer, sizeof(log_buffer));
	HANDLE_EINTR(::write(log->fds.fd, log_buffer, n));
}
#endif
} //end namespace xr