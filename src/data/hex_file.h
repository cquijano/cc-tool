/*
 * hex_file.h
 *
 * Created on: Jul 15, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * License: GNU GPL v2
 *
 */

#ifndef _HEX_FILE_H_
#define _HEX_FILE_H_

#include "common.h"
#include "data_section_store.h"

class HexFile
{
public:
	static DataSectionStore& load(const String &file_name,
			DataSectionStore &section_store, bool ignore_crc_mismatch = false);

	static void save(const String &file_name, const DataSectionStore &section_store);
};

void hex_file_load(const String &file_name, DataSectionStore &section_store, bool ignore_crc_mismatch = false);
void hex_file_save(const String &file_name, const DataSectionStore &section_store);

#endif // !_HEX_FILE_H_
