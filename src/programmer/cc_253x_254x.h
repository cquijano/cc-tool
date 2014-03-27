/*
 * cc_253x_254x.h
 *
 * Created on: Aug 4, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * License: GNU GPL v2
 *
 */

#ifndef _CC_253X_254X_H_
#define _CC_253X_254X_H_

#include "cc_debug_interface.h"
#include "cc_unit_driver.h"

class CC_253x_254x : public CC_UnitDriver
{
public:
	virtual void supported_units(Unit_ID_List &units);
	virtual void find_unit_info(UnitInfo &info);
	virtual void read_info_page(ByteVector &info_page);
	virtual bool erase_check_comleted();

	/// @param offset must be at page boundaries
	virtual void mac_address_read(size_t index, ByteVector &mac_address);
	virtual void flash_write(const DataSectionStore &sections);
	virtual bool config_write(const ByteVector &mac_address,
			const ByteVector &lock_data);

	virtual bool flash_image_embed_mac_address(DataSectionStore &sections,
			const ByteVector &mac_address);
	virtual bool flash_image_embed_lock_data(DataSectionStore &sections,
			const ByteVector &lock_data);

	virtual void convert_lock_data(const StringVector& qualifiers,
			ByteVector& lock_data);

	CC_253x_254x(USB_Device &programmer, ProgressWatcher &pw);

private:
	virtual void flash_read_block(size_t offset, size_t size, ByteVector &flash_data);

//	void flash_read_page(uint16_t address, ByteVector &flash_data);
	void flash_select_bank(uint_t bank);
//	void flash_read_start();
//	void flash_read_end();

	/// Return absolute flash offset where secondary mac address is stored
	uint_t mac_address_offset() const;
	/// Return absolute flash offset where lock data is stored
	uint_t lock_data_offset() const;
};

#endif // !_CC_253X_2540_H_
