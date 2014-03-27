/*
 * data_section.cpp
 *
 * Created on: Jul 29, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * License: GNU GPL v2
 *
 */

#include "data_section.h"

//==============================================================================
std::ostream& operator <<(std::ostream &os, const DataSection &o)
{
	os << std::uppercase << std::hex << std::setfill('0');

	os << "start address: " << std::setw(8)	<< o.address << ", ";
	os << "end address: " << std::setw(8) << o.next_address() << ", ";
	os << "size: " << std::setw(8) << o.size();

	return os;
}

//==============================================================================
bool DataSection::empty() const
{
	return data.empty();
}

//==============================================================================
size_t DataSection::size() const
{
	return data.size();
}

//==============================================================================
uint_t DataSection::next_address() const
{
	return address + data.size();
}

//==============================================================================
DataSection::DataSection() :
		address(0)
{ }

//==============================================================================
DataSection::DataSection(uint_t address_, const ByteVector &data_) :
		address(address_),
		data(data_)
{ }

//==============================================================================
DataSection::DataSection(uint_t address_, const uint8_t data_[], size_t size) :
		address(address_),
		data(data_, data_ + size)
{ }
