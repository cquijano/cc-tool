/*
 * cc_unit_driver.h
 *
 * Created on: Aug 4, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * License: GNU GPL v2
 *
 */

#ifndef _CC_UNIT_DRIVER_H_
#define _CC_UNIT_DRIVER_H_

#include "data/data_section_store.h"
#include "data/progress_watcher.h"
#include "usb/usb_device.h"
#include "cc_unit_info.h"

const size_t FLASH_EMPTY_BYTE 	   		= 0xFF;
const size_t XDATA_READ_CHUNK_SIZE 		= 128;
const size_t FLASH_READ_CHUNK_SIZE 		= 128;
const size_t FLASH_BANK_SIZE 			= 1024 * 32;
const size_t FLASH_MAPPED_BANK_OFFSET 	= 1024 * 32;

const uint8_t FCTL_BUSY					= 0x80;
const uint8_t FCTL_ABORT				= 0x20;
const uint8_t FCTL_ERASE				= 0x01;

struct USB_DeviceID;

class CC_UnitDriver : boost::noncopyable
{
public:
	virtual void supported_units(Unit_ID_List &units) = 0;
	virtual void find_unit_info(UnitInfo &info) = 0;

	virtual void read_info_page(ByteVector &info_page);

	/// Read mac address
	/// @param index 0 - primary address, 1 - secondary address (if any)
	virtual void mac_address_read(size_t index, ByteVector &mac_address);

	/// Write and verify lock data and/or mac address. If field is empty
	/// it wont be written.
	/// @return false if varification after write failed
	virtual bool config_write(const ByteVector &mac_address,
			const ByteVector &lock_data) = 0;

	/// Integrate mac address into flash image
	/// @return true if target store mac address in flash
	virtual bool flash_image_embed_mac_address(DataSectionStore &sections,
			const ByteVector &mac_address);

	/// Integrate lock data into flash image
	/// @return true if target store lock data in flash
	virtual bool flash_image_embed_lock_data(DataSectionStore &sections,
			const ByteVector &lock_data);

	/// Convert list of string qulifiers like 'debug', 'pages:xx' into lock data
	virtual void convert_lock_data(const StringVector& qualifiers,
			ByteVector& lock_data) = 0;

	/// Erase flash completely. Operation is asynchronious,
	virtual void erase();
	virtual bool erase_check_comleted() = 0;

	/// Erase single page. Page size depends on target
	/// @param page_offset must be aligned to a page boundary.
	/// @return false if erase was aborted (e.g. 'cause page is locked)
	virtual bool erase_page(uint_t page_offset);

	/// Write data into flash. Empty data block are skipped.
	/// Modified parts of flash should be bllank.
	virtual void flash_write(const DataSectionStore &sections) = 0;

	/// Compare specified data to data from flash. Empty blocks are skipped.
	/// @return false if verification failed
	virtual bool flash_verify_by_crc(const DataSectionStore &sections);

	/// Compare specified data to data from flash. Empty blocks are skipped.
	/// @return false if verification failed
	virtual bool flash_verify_by_read(const DataSectionStore &sections);

	/// prepare reading procedure
	void flash_read_start();

	/// finish reading procedure
	void flash_read_end();

	/// read flash block (offset and size are not limited by any boundares\banks)
	/// @param offset absolute flash offset
	virtual void flash_read_block(size_t offset, size_t size, ByteVector &data) = 0;

	void read_debug_status(uint8_t &status);
	void read_debug_config(uint8_t &config);
	void write_debug_config(uint8_t config);

	void read_sfr(uint8_t address, uint8_t &value);
	void write_sfr(uint8_t address, uint8_t value);

	void write_xdata_memory(uint16_t address, const ByteVector &data);
	void write_xdata_memory(uint16_t address, const uint8_t data[], size_t size);
	void write_xdata_memory(uint16_t address, uint8_t data);

	void read_xdata_memory(uint16_t address, size_t count, ByteVector &out);
	uint8_t read_xdata_memory(uint16_t address);

	/// @param debug_mode if true after reset target will be halted
	void reset(bool debug_mode);

	uint_t lock_data_size() const;

	bool set_flash_size(uint_t flash_size);
	void set_programmer_ID(const USB_DeviceID& programmer_ID);

	CC_UnitDriver(USB_Device &programmer, ProgressWatcher &pw);

protected:
	void select_info_page_flash(bool select_info_page);

	/// Read data from the 16-bit address space. Bank number is not changed.
	/// Result data will be appended to the end of data
	void flash_read_near(uint16_t address, size_t size, ByteVector &data);

	/// Read any block size
	void flash_read(size_t offset, size_t size, ByteVector &data);//todo: rename!!!

	void set_reg_info(const UnitCoreInfo &);
	UnitCoreInfo get_reg_info();

	void write_flash_slow(const DataSectionStore &sections);

	//void write_flash_word(const DataSectionStore &sections);
	void write_lock_to_info_page(uint8_t lock_byte);
	//void

	/// @return true if specified memory block contains only 1s ()
	bool empty_block(const uint8_t* data, size_t size);

	/// Convert list of string qulifiers 'debug', 'flash:xx', 'boot'
	void convert_lock_data_std_set(
			const StringVector& qualifiers,
			const ByteVector& lock_sizes,
			ByteVector& lock_data);

	USB_Device &usb_device_;
	ProgressWatcher &pw_;
	UnitInfo unit_info_;

	uint8_t endpoint_in_;
	uint8_t endpoint_out_;

private:
	uint16_t calc_block_crc();

	UnitCoreInfo reg_info_;
};

typedef boost::shared_ptr<CC_UnitDriver> CC_UnitDriverPtr;
typedef std::list<CC_UnitDriverPtr> CC_UnitDriverPtrList;

#endif // !_CC_UNIT_DRIVER_H_

