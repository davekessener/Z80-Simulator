#include "Timer.h"

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

void Timer::reset(void)
{
	t_ = clock_t::now();
}

Timer::time_t Timer::get(void)
{
	return std::chrono::duration_cast<time_t>(clock_t::now() - t_);
}

Timer::time_t Timer::elapsed(void)
{
	time_t r = get();

	reset();

	return r;
}

void Timer::sync(time_t t)
{
	time_t r = get();

	if(t > r) sleep(t - r);

	reset();
}

void Timer::sleep(time_t t)
{
	uint32_t total = t.count();
	point_t t1 = clock_t::now();

	while(total >= 900000)
	{
		usleep(900000);
		total = (t - std::chrono::duration_cast<time_t>(clock_t::now() - t1)).count();
	}

	usleep(total);
}

