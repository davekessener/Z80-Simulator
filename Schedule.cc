#include "Schedule.h"

namespace lib {

void Schedule::schedule(run_fn cb, uint64_t f)
{
	Timer t;

	t.reset();
	callbacks_.push_back(std::make_pair(std::make_pair(f, 0ul), std::make_pair(t, cb)));
}

void Schedule::step(void)
{
	Timer t;
	uint64_t c = (uint64_t)-1, e;

	t.reset();

	for(auto& p : callbacks_)
	{
		uint64_t r = p.first.second + p.second.first.elapsed().count();

		if(r >= p.first.first)
		{
			r -= p.first.first;
			c = 0;
			p.second.second();
		}
		else if(c > 0 && r < c)
		{
			c = r;
		}

		p.first.second = r;
	}

	e = t.elapsed().count();

	if(c > 0 && c > e)
	{
		t.sleep(Timer::time_t(c - e));
	}
}

}

