/*
 * data_section.h
 *
 * Created on: Jul 29, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * License: GNU GPL v2
 *
 */

#ifndef _DATA_SECTION_H_
#define _DATA_SECTION_H_

#include <list>
#include "common.h"

struct DataSection
{
	bool empty() const;
	size_t size() const;
	uint_t next_address() const;

	DataSection();
	DataSection(uint_t address, const ByteVector &data);
	DataSection(uint_t address, const uint8_t data[], size_t size);

	uint_t address;
	ByteVector data;
};

typedef std::list<DataSection> DataSectionList;

std::ostream& operator <<(std::ostream &os, const DataSection &o);

#endif // !_DATA_SECTION_H_
