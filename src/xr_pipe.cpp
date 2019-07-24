#include "xr_pipe.h"

#include <xr_log.h>
#include <xr_file.h>

namespace xr{
pipe_t::pipe_t()
{
	::memset(this->handles, 0, sizeof(this->handles));
}

int pipe_t::create()
{
	if (FAIL == ::pipe(this->handles)){
		BOOT_LOG(FAIL, "PIPE CREATE FAILED [err:%s]", strerror(errno));
		return FAIL;
	}

	file_t::fcntl_add_flag(this->handles[E_PIPE_INDEX_RDONLY], O_NONBLOCK | O_RDONLY);
	file_t::fcntl_add_flag(this->handles[E_PIPE_INDEX_WRONLY], O_NONBLOCK | O_WRONLY);

	// 这里设置为FD_CLOEXEC表示当程序执行exec函数时本fd将被系统自动关闭,表示不传递给exec创建的新进程
	file_t::fcntl_add_flag(this->handles[E_PIPE_INDEX_RDONLY], FD_CLOEXEC);
	file_t::fcntl_add_flag(this->handles[E_PIPE_INDEX_WRONLY], FD_CLOEXEC);

	return SUCC;
}

int pipe_t::close( int mode )
{
	if (mode < E_PIPE_INDEX_RDONLY || E_PIPE_INDEX_MAX <= mode){
		return FAIL;
	}
	return file_t::close_fd(this->handles[mode]);
}

int pipe_t::read_fd()
{
	return this->handles[E_PIPE_INDEX_RDONLY];
}

int pipe_t::write_fd()
{
	return this->handles[E_PIPE_INDEX_WRONLY];
}
}//end namespace xr