#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <unistd.h>

class Timer
{
	public:
	typedef std::chrono::high_resolution_clock clock_t;
	typedef std::chrono::microseconds time_t;
	typedef clock_t::time_point point_t;

	public:
		void reset( );
		time_t get( );
		time_t elapsed( );
		void sync(time_t);
		void sleep(time_t);
	private:
		point_t t_;
};

#endif

