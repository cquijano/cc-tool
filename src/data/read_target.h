/*
 * read_traget.h
 *
 * Created on: Oct 31, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * License: GNU GPL v2
 *
 */

#ifndef _READ_TARGET_H_
#define _READ_TARGET_H_

#include "common.h"

struct OptionFileInfo
{
	String type;
	String name;
	size_t offset;
};

void option_extract_file_info(const String &input, OptionFileInfo &file_info,
		bool support_offset);

class ReadTarget
{
public:
	enum SourceType { ST_CONSOLE, ST_FILE };

	SourceType source_type() const;
	void set_source(const String &input);
	void on_read(const ByteVector &data) const;

	ReadTarget();

private:
	String file_format_;
	String file_name_;
	SourceType source_type_;
};

#endif // !_READ_TARGET_H_
