#ifndef LIB_SCHEDULE_H
#define LIB_SCHEDULE_H

#include <vector>
#include <functional>

#include "Timer.h"

namespace lib
{
	class Schedule
	{
		public:
		typedef std::function<void(void)> run_fn;

		public:
			void schedule(run_fn, uint64_t);
			void step( );
		private:
			std::vector<std::pair<std::pair<uint64_t, uint64_t>, std::pair<Timer, run_fn>>> callbacks_;
	};
}

#endif

