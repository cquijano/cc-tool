/*
 * cc_251x_111x.h
 *
 * Created on: Nov 9, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * License: GNU GPL v2
 *
 */

#ifndef _CC_251X_111X_H_
#define _CC_251X_111X_H_

#include "cc_unit_driver.h"

class CC_251x_111x : public CC_UnitDriver
{
public:
	virtual void supported_units(Unit_ID_List &units);
	virtual void find_unit_info(UnitInfo &info);

	virtual bool erase_check_comleted();

	virtual void flash_write(const DataSectionStore &sections);
	virtual void flash_read_block(size_t offset, size_t size, ByteVector &data);

	virtual bool config_write(const ByteVector &mac_address, const ByteVector &lock_data);

	virtual void convert_lock_data(const StringVector& qualifiers,
			ByteVector& lock_data);

	CC_251x_111x(USB_Device &programmer, ProgressWatcher &pw);
};

#endif // !_CC_251X_111X_H_
