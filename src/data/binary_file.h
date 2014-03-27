/*
 * binary_file.h
 *
 * Created on: Jul 30, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 *    License: GNU GPL v2
 */

#ifndef _BINARY_FILE_H_
#define _BINARY_FILE_H_

#include "common.h"

class BinaryFile
{
public:
	static ByteVector& load(const String &file_name, ByteVector &vector);
	static void save(const String &file_name, const ByteVector &vector);
};

void binary_file_load(const String &file_name, ByteVector &vector);
void binary_file_save(const String &file_name, const ByteVector &vector);

#endif // !_BINARY_FILE_H_
