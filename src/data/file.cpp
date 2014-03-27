/*
 * file.cpp
 *
 * Created on: Aug 4, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * License: GNU GPL v2
 *
 */

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "file.h"

//==============================================================================
inline void check_open(const String &context, FILE *file)
{
	if (!file)
		throw FileException(context + " on not opened file");
}

//==============================================================================
inline void file_io_error(const String &context, const String &file_name)
{
	String error = context + " failed, file name: "  + file_name + ": " + strerror(errno);

	throw FileException(error);
}

//==============================================================================
File::File() :
	file_(NULL)
{ }

//==============================================================================
File::~File()
{
	if (file_ != NULL)
		fclose(file_);
}

//==============================================================================
void File::open(const String &file_name, const char mode[], off_t max_size)
{
	close();

	file_name_ = file_name;

	file_ = fopen(file_name.c_str(), mode);
	if (file_ == NULL)
		file_io_error("File::open", file_name_);

	if (max_size && max_size < size())
	{
		close();
		throw FileException("File " + file_name + " is too long");
	}
}

//==============================================================================
void File::close()
{
	if (file_ != NULL)
		if (fclose(file_) < 0)
			file_io_error("File::close", file_name_);
	file_ = NULL;
}

//==============================================================================
off_t File::size()
{
	check_open("File::size", file_);

	struct stat file_stat;
	if (fstat(fileno(file_), &file_stat) != 0)
		file_io_error("File::size", file_name_);

	return file_stat.st_size;
}

//==============================================================================
off_t File::seek(off_t offset, uint_t position)
{
	check_open("File::seek", file_);

	off_t result = fseeko(file_, offset, position);

	if (result == (off_t) -1)
		file_io_error("File::seek", file_name_);
	return result;
}

//==============================================================================
void File::write(const uint8_t data[], size_t size)
{
	check_open("File::write", file_);

	if (fwrite(data, size, 1, file_) != 1 || ferror(file_))
		file_io_error("File::write", file_name_);
}

//==============================================================================
void File::write(const ByteVector &data)
{
	check_open("File::write", file_);

	if (fwrite(&data[0], data.size(), 1, file_) != 1 || ferror(file_))
		file_io_error("File::write", file_name_);
}

//==============================================================================
void File::read(uint8_t data[], size_t size)
{
	check_open("File::read", file_);

	if (fread(data, size, 1, file_) != 1 || ferror(file_))
		file_io_error("File::read", file_name_);
}

//==============================================================================
void File::read(ByteVector &data, size_t size)
{
	check_open("File::read", file_);

	data.resize(size);

	if (fread(&data[0], data.size(), 1, file_) != 1 || ferror(file_))
		file_io_error("File::read", file_name_);
}

//==============================================================================
bool File::read_n(uint8_t data[], size_t max_size, size_t &read_size)
{
	check_open("File::read_max", file_);

	ssize_t result = fread(data, 1, max_size, file_);
	if (ferror(file_))
		file_io_error("File::read_max", file_name_);

	read_size = result;
	return feof(file_);
}
