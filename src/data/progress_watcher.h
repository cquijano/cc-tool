/*
 * progress_watcher.h
 *
 * Created on: Nov 12, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * License: GNU GPL v2
 *
 */

#ifndef _PROGRESS_WATCHER_H_
#define _PROGRESS_WATCHER_H_

#include "common.h"
#include <boost/signals2.hpp>

class ProgressWatcher : boost::noncopyable
{
public:
	typedef boost::signals2::signal<void (uint_t done_size, uint_t full_size)> OnProgress;

	void do_on_read_progress(const OnProgress::slot_type &slot);
	void do_on_write_progress(const OnProgress::slot_type &slot);

	void read_progress(uint_t done_chunk_size);
	void write_progress(uint_t done_chunk_size);

	void read_start(size_t total_size);
	void read_finish();

	void write_start(size_t total_size);
	void write_finish();

	void enable(bool enable);

	ProgressWatcher();

private:
	uint_t total_read_;
	uint_t total_write_;
	bool read_started_;
	bool write_started_;
	uint_t done_read_;
	uint_t done_write_;
	bool enabled_;
	OnProgress on_read_progress_;
	OnProgress on_write_progress_;
};

#endif // !_PROGRESS_WATCHER_H_
