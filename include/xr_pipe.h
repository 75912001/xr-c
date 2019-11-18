#pragma once

namespace xr
{
enum
{
	E_PIPE_INDEX_RDONLY = 0,
	E_PIPE_INDEX_WRONLY = 1,
	E_PIPE_INDEX_MAX,
};

struct pipe_t
{
	pipe_t();
	int create();
	int close(int mode);
	int read_fd();
	int write_fd();

private:
	int handles[E_PIPE_INDEX_MAX];
};
} // namespace xr