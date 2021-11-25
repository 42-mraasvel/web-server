#pragma once

# include <ctime>

class Timer
{
	public:
		Timer();
		double elapsed() const;
		void reset();
	private:
		time_t start;
};
