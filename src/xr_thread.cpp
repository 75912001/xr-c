#include "xr_thread.h"
#include "xr_log.h"
#include "xr_util.h"

namespace
{
void *thread_fun(void *pdata)
{
	xr::thread_t *pthread = (xr::thread_t *)pdata;
	while (SUCC != pthread->do_work_begin())
	{
	}

	while (pthread->is_run)
	{
		pthread->do_work_fn(pthread->data);
	}

	while (SUCC != pthread->do_work_end())
	{
	}
	return NULL;
}
} // namespace

namespace xr
{
int thread_t::start(void *data)
{
	this->data = data;

	int r = ::pthread_create(&this->id, NULL, thread_fun, (void *)this);

	if (0 != r)
	{
		ALERT_LOG("[err_code:%d, err:%s]", errno, strerror(errno));
		return r;
	}
	return SUCC;
}

thread_t::thread_t()
{
	this->id = 0;
	this->is_run = true;
	this->data = NULL;
}

thread_t::~thread_t()
{
	this->is_run = false;
	::pthread_join(this->id, NULL);
	::pthread_detach(this->id);
}

int thread_t::stop()
{
	this->is_run = false;

	return SUCC;
}

int thread_t::do_work_begin()
{
	return SUCC;
}

int thread_t::do_work_end()
{
	return SUCC;
}
} //end namespace xr