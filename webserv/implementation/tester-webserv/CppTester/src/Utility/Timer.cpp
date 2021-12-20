#include "Timer.hpp"

Timer::Timer()
{
	time(&start);
}

double Timer::elapsed() const
{
	time_t end;
	time(&end);
	return difftime(end, start);
}

void Timer::reset()
{
	time(&start);
}
