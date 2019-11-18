#include "xr_file.h"
#include "xr_util.h"

namespace xr
{
int file_t::get_dir_file(const char *path, std::vector<std::string> &file_names)
{
	DIR *dirp;
	dirp = ::opendir(path);
	if (NULL != dirp)
	{
		while (1)
		{
			struct dirent *direntp;
			direntp = ::readdir(dirp);
			if (NULL == direntp)
			{
				break;
			}
			if (DT_REG == direntp->d_type)
			{
				file_names.push_back(direntp->d_name);
			}
		}
		::closedir(dirp);
	}
	else
	{
		return FAIL;
	}
	return SUCC;
}

int file_t::set_io_block(int fd, bool is_block)
{
	int val;
	if (is_block)
	{
		val = (~O_NONBLOCK & ::fcntl(fd, F_GETFL));
	}
	else
	{
		val = (O_NONBLOCK | ::fcntl(fd, F_GETFL));
	}
	return ::fcntl(fd, F_SETFL, val);
}

int file_t::fcntl_add_flag(int fd, int add_flag)
{
	int flag = ::fcntl(fd, F_GETFD, 0);
	flag |= add_flag;
	return ::fcntl(fd, F_SETFD, flag);
}

int file_t::close_fd(int &fd)
{
	int ret = SUCC;
	if (INVALID_FD != fd)
	{
		ret = HANDLE_EINTR(::close(fd));
		if (ERR != ret)
		{
			fd = INVALID_FD;
		}
	}
	return ret;
}

int file_t::shutdown_rd(int fd)
{
	int ret = SUCC;
	if (INVALID_FD != fd)
	{
		ret = ::shutdown(fd, SHUT_RD);
	}
	return ret;
}
} //end namespace xr