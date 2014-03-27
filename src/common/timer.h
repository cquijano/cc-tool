/*
 * timer.h
 *
 * Created on: Nov 5, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * License: GNU GPL v2
 *
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#include "common.h"

class Timer : boost::noncopyable
{
public:
	String elapsed_time() const;
	Timer();

private:
	void start();

	uint_t start_time_;
};

#endif // !_TIMER_H_
