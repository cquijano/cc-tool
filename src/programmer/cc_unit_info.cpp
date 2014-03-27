/*
 * cc_uint_info.cpp
 *
 * Created on: Nov 7, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * License: GNU GPL v2
 *
 */

#include "cc_unit_info.h"

//==============================================================================
UnitInfo::UnitInfo() :
		ID(0),
		internal_ID(0),
		revision(0),
		flags(0),
		flash_size(0),
		max_flash_size(0),
		ram_size(0),
		mac_address_count(0),
		mac_address_size(0),
		flash_page_size(0)
{ }

//==============================================================================
UnitCoreInfo::UnitCoreInfo()
{	memset(this, 0, sizeof(UnitCoreInfo)); }

//==============================================================================
size_t UnitInfo::actual_flash_size() const
{	return 	1024 * (flash_size ? flash_size : max_flash_size); }

#define CHECK_PARAM(x) check_param(x, __FILE__, __LINE__)

//==============================================================================
void check_param(bool assert, const String& module, uint_t line)
{
	if (assert)
		return;

	throw std::runtime_error("internal error at " + module + ":" + number_to_string(line));
}
