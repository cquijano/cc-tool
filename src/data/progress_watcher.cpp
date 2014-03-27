/*
 * progress_watcher.cpp
 *
 * Created on: Nov 12, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * License: GNU GPL v2
 *
 */

#include "progress_watcher.h"

//==============================================================================
void ProgressWatcher::do_on_read_progress(const OnProgress::slot_type &slot)
{	on_read_progress_.connect(slot); }

//==============================================================================
void ProgressWatcher::do_on_write_progress(const OnProgress::slot_type &slot)
{	on_write_progress_.connect(slot); }

//==============================================================================
void ProgressWatcher::read_progress(uint_t done_chunk)
{
	if (enabled_ && read_started_ && done_chunk)
	{
		done_read_ += done_chunk;
		on_read_progress_(done_read_, total_read_);
	}
}

//==============================================================================
void ProgressWatcher::write_progress(uint_t done_chunk)
{
	if (enabled_ && write_started_ && done_chunk)
	{
		done_write_ += done_chunk;
		on_write_progress_(done_write_, total_write_);
	}
}

//==============================================================================
void ProgressWatcher::read_start(size_t total_size)
{
	if (!enabled_)
		return;

	read_started_ = true;
	total_read_ = total_size;
	done_read_ = 0;
}

//==============================================================================
void ProgressWatcher::read_finish()
{
	if (!enabled_)
		return;

	read_started_ = false;
	if (done_read_ != total_read_)
		on_read_progress_(total_read_, total_read_);
}

//==============================================================================
void ProgressWatcher::write_start(size_t total_size)
{
	if (!enabled_)
		return;

	write_started_ = true;
	total_write_ = total_size;
	done_write_ = 0;
}

//==============================================================================
void ProgressWatcher::write_finish()
{
	if (!enabled_)
		return;

	write_started_ = false;
	if (done_write_ != total_write_)
		on_write_progress_(done_write_, total_write_);
}

//==============================================================================
void ProgressWatcher::enable(bool enable)
{	enabled_ = enable; }

//==============================================================================
ProgressWatcher::ProgressWatcher() :
		total_read_(0),
		total_write_(0),
		read_started_(false),
		write_started_(false),
		done_read_(0),
		done_write_(0),
		enabled_(true)
{ }
