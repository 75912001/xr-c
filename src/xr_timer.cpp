#include "xr_timer.h"

//4,8,16,32,64
#define GEN_TIME_INTERVAL(__idx__) (1 << ((__idx__) + 2))

namespace xr{
timer_t* g_timer = NULL;
void timer_t::scan()
{
	//microseconds timer
	this->scan_micro();

	//second timer
	if (this->last_handle_sec != (int32_t)this->now_sec()) {
		this->last_handle_sec = this->now_sec();
		this->scan_sec();
	}
}

timer_t::sec_t* timer_t::add_sec( list_head_t* head, 
	ON_TIMER_FUN func, void* owner, void* data, ::time_t expire )
{
	sec_t* timer = (sec_t*)::malloc(sizeof(sec_t));

	timer->init();
	timer->function  = func;
	timer->expire    = expire;
	timer->owner     = owner;
	timer->data      = data;

	list_add_tail(&timer->sprite_list, head);
	this->add_sec(timer);
	return timer;
}

void timer_t::rm_sec( sec_t* t)
{
	if (0 != t->sprite_list.next) {
		list_del(&t->sprite_list);
	}
	list_del(&t->entry);
	free(t);
}

void timer_t::rm_all_sec( list_head_t* head )
{
	sec_t *t;
	list_head_t *l, *m;

	list_for_each_safe (l, m, head) {
		t = list_entry (l, sec_t, sprite_list);
		this->rm_sec(t);
	}
}

timer_t::micro_t* timer_t::add_micro( ON_TIMER_FUN func,
	const struct timeval* tv, void* owner, void* data )
{
	micro_t* timer = (micro_t*)::malloc(sizeof(micro_t));

	timer->init();
	timer->function  = func;
	timer->tv        = *tv;
	timer->owner     = owner;
	timer->data      = data;

	list_add_tail(&timer->entry, &this->micro_timer);
	return timer;
}

void timer_t::rm_micro( micro_t *t )
{
	list_del_init(&t->entry);
	free(t);
}

void timer_t::rm_all_micro( void* owner )
{
	list_head_t *l, *p;
	micro_t* t;

	list_for_each_safe(l, p, &this->micro_timer) {
		t = list_entry(l, micro_t, entry);
		if (t->owner == owner) {
			this->rm_micro(t);
		}
	}
}

void timer_t::renew_sec_timer_list( const int idx )
{
	time_t min_exptm = 0;
	list_head_t* cur;
	list_head_t* next;

	list_for_each_safe(cur, next, &this->vec[idx].head) {
		sec_t* t = list_entry(cur, sec_t, entry);
		int i = this->find_min_idx(t->expire - this->now_sec(), idx);
		if (i != idx) {
			list_del(&t->entry);
			list_add_tail(&t->entry, &this->vec[i].head);
			this->set_min_exptm(t->expire, i);
		} else if ((t->expire < min_exptm) || 0 == min_exptm) {
			min_exptm = t->expire;
		}
	}

	this->vec[idx].min_expiring_time = min_exptm;
}

void timer_t::scan_micro()
{
	list_head_t *l, *p;
	micro_t* t;

	list_for_each_safe(l, p, &this->micro_timer) {
		t = list_entry(l, micro_t, entry);
		if (NULL == (t->function)) {
			this->rm_micro(t);
		} else if (this->now.tv_sec > t->tv.tv_sec 
			|| (this->now.tv_sec == t->tv.tv_sec && this->now.tv_usec > t->tv.tv_usec)) {
			if (0 == t->function(t->owner, t->data)) {
				this->rm_micro(t);
			}
		}
	}
}

void timer_t::scan_sec()
{
	list_head_t *l, *p;
	sec_t* t;

	list_for_each_safe(l, p, &this->vec[0].head) {
		t = list_entry(l, sec_t, entry);
		if (NULL == (t->function)){
			this->rm_sec(t);
		} else if (t->expire <= (int32_t)this->now_sec()) {
			if (0 == t->function(t->owner, t->data)) {
				this->rm_sec(t);
			}
		}
	}

	for (int i = 1; i != E_TIMER_VEC_SIZE; ++i) {
		if ((this->vec[i].min_expiring_time - this->now_sec()) < (uint32_t)GEN_TIME_INTERVAL(i)) {
			this->renew_sec_timer_list(i);
		}
	}
}

timer_t::timer_t()
{
	this->renew_time();
	this->last_handle_sec = 0;
	for (int i = 0; i < E_TIMER_VEC_SIZE; i++) {
		this->vec[i].expire = GEN_TIME_INTERVAL(i);
	}
	INIT_LIST_HEAD(&this->micro_timer);
}

timer_t::~timer_t()
{
	list_head_t* l;
	list_head_t* p;

	for (int i = 0; i < E_TIMER_VEC_SIZE; i++) {
		list_for_each_safe(l, p, &this->vec[i].head) {
			sec_t* t = list_entry(l, sec_t, entry);
			this->rm_sec(t);
		}	
	}

	list_for_each_safe(l, p, &this->micro_timer) {
		micro_t* t = list_entry(l, micro_t, entry);
		this->rm_micro(t);
	}
}

timer_event_t::~timer_event_t()
{
	this->remove_all();
}

timer_event_t::timer_event_t( timer_t* timer )
{
	this->timer = timer;
	INIT_LIST_HEAD(&this->timer_list);
}

timer_t::sec_t* timer_event_t::add_sec( ON_TIMER_FUN func,
	void* data, ::time_t expire)
{
	return this->timer->add_sec(&this->timer_list, func, this, data, expire);
}

void timer_event_t::remove_all()
{
	this->timer->rm_all_sec(&(this->timer_list));
	this->timer->rm_all_micro(this);
}

void timer_event_t::remove_micro( timer_t::micro_t* timer_micro )
{
	this->timer->rm_micro(timer_micro);
}

void timer_event_t::remove_sec( timer_t::sec_t* timer_sec )
{
	this->timer->rm_sec(timer_sec);
}

timer_t::micro_t* timer_event_t::add_micro( ON_TIMER_FUN func, void* data, timeval* expire )
{
	return this->timer->add_micro(func, expire, this, data);
}

void timer_t::sec_t::init()
{
	INIT_LIST_HEAD(&this->sprite_list);
	INIT_LIST_HEAD(&this->entry);
	this->expire = 0;
	this->owner = NULL;
	this->data = NULL;
	this->function = NULL;
}

void timer_t::micro_t::init()
{
	INIT_LIST_HEAD(&this->entry);
	this->tv.tv_sec = 0;
	this->tv.tv_usec = 0;
	this->owner = NULL;
	this->data = NULL;
	this->function = NULL;
}

timer_t::tvec_root_s::tvec_root_s()
{
	INIT_LIST_HEAD(&this->head);
	this->expire = 0;
	this->min_expiring_time = 0;
}
}//end namespace xr