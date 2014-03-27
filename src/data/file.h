/*
 * file.h
 *
 * Created on: Aug 1, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * License: GNU GPL v2
 *
 */

#ifndef _FILE_H_
#define _FILE_H_

#include "common.h"

class File //: boost::noncopyable
{
public:
	void open(const String &file_name, const char mode[], off_t max_size = 0); // throw
	void close(); // throw

	off_t size(); // throw
	off_t seek(off_t offset, uint_t position); // throw

	void write(const uint8_t data[], size_t size); // throw
	void write(const ByteVector &data); // throw

	void read(uint8_t data[], size_t size); // throw
	void read(ByteVector &data, size_t size); // throw
	// return false if eof
	bool read_n(uint8_t data[], size_t max_size, size_t &read_size); // throw

	File();
	~File();

private:
	FILE *file_;
	String file_name_;
};

class FileException : public std::runtime_error
{
public:
	FileException(const String& arg) : std::runtime_error(arg) { }
};

#endif // !_FILE_H_
