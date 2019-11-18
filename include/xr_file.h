//文件相关操作

#pragma once

#include "xr_include.h"

namespace xr
{
struct file_t
{
	//set the given fd to be blocking or noblocking
	//returns:0:success.-1:error
	//fd:file descriptor to be set
	//is_block:true:blocking.false:nonblocking
	static int set_io_block(int fd, bool is_block);
	static int fcntl_add_flag(int fd, int add_flag);

	//close the given fd(if fd is VALID),and set to be -1
	//returns:0:success.-1:error
	//其中判定 INVALID_FD != fd  时才做关闭操作
	static int close_fd(int &fd);

	//Disables further receive operations.关闭socket可读
	//returns:0:success.-1:error
	//其中判定 INVALID_FD != fd  时才做关闭操作
	//************************************
	static int shutdown_rd(int fd);

	//得到目录下的文件
	//returns:0:OK.其它:失败
	//path 目录的路径
	// file_names  返回的文件名
	static int get_dir_file(const char *path, std::vector<std::string> &file_names);
};

} //end namespace xr