/*
 * cc_253x_254x.cpp
 *
 * Created on: Aug 4, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * License: GNU GPL v2
 *
 */

#include <boost/regex.hpp>
#include "log.h"
#include "cc_253x_254x.h"

#include "data/hex_file.h"
#include "data/binary_file.h"

const size_t LOCK_DATA_SIZE 	= 16;
const size_t MAX_PAGE_COUNT 	= LOCK_DATA_SIZE * 8;
const size_t INFO_PAGE_OFFSET 	= 0x7800;
const size_t INFO_PAGE_SIZE 	= 0x800;

const uint16_t XREG_DBGDATA 	= 0x6260;
const uint16_t XREG_FWDATA 		= 0x6273;
const uint16_t XREG_FCTL 		= 0x6270;
const uint16_t XREG_FADDRL 		= 0x6271;
const uint16_t XREG_FADDRH 		= 0x6272;
const uint16_t XREG_DMA1CFGL 	= 0x70D2;
const uint16_t XREG_DMA1CFGH 	= 0x70D3;
const uint16_t XREG_DMAARM 		= 0x70D6;
const uint16_t XREG_DMAREQ 		= 0x70D7;
const uint16_t XREG_DMAIRQ 		= 0x70D1;

const uint16_t XREG_MEMCTR 		= 0x70C7;
const uint16_t XREG_FMAP 		= 0x709F;

//==============================================================================
static void read_range(const String& input, BoolVector& range,
		uint_t min_value, uint_t max_value)
{
	boost::cmatch what;
	boost::regex exp("(\\d{1,})-(\\d{1,})");

	StringVector list;
	boost::split(list, input, boost::is_any_of(","));

	foreach(const String& item, list)
	{
		uint_t n = 0;
		uint_t r1, r2;
		if (string_to_number(item, n))
			r1 = r2 = n;
		else
		{
			if (!boost::regex_match(item.c_str(), what, exp))
				throw std::runtime_error("read_range: incorrect input");
			string_to_number(what[1].str(), r1);
			string_to_number(what[2].str(), r2);
			if (r1 > r2)
				std::swap(r1, r2);
		}
		if (r2 > max_value || r1 < min_value)
			throw std::runtime_error("read_range: value is out of range");

		if (range.size() <= r2)
			range.resize(r2 + 1, false);
		for (; r1 <= r2; r1++)
			range[r1] = true;
	}
}

//==============================================================================
CC_253x_254x::CC_253x_254x(USB_Device &usb_device, ProgressWatcher &pw) :
		CC_UnitDriver(usb_device, pw)
{
	UnitCoreInfo reg_info;

	reg_info.lock_size 			= 16;
	reg_info.flash_word_size	= 4;
	reg_info.write_block_size	= 1024;
	reg_info.verify_block_size	= 1024;
	reg_info.xbank_offset		= 0x8000;
	reg_info.dma0_cfg_offset	= 0x0800;
	reg_info.dma_data_offset	= 0x0000;

	reg_info.memctr		= XREG_MEMCTR;
	reg_info.fmap		= XREG_FMAP;
	reg_info.faddrh		= XREG_FADDRH;
	reg_info.faddrl		= XREG_FADDRL;
	reg_info.fctl		= XREG_FCTL;
	reg_info.fwdata		= XREG_FWDATA;
	reg_info.rndh		= 0x70BD;
	reg_info.rndl		= 0x70BC;
	reg_info.dma0_cfgh	= 0x70D5;
	reg_info.dma0_cfgl	= 0x70D4;
	reg_info.dma_arm	= XREG_DMAARM;
	reg_info.dma_req	= XREG_DMAREQ;
	reg_info.dma_irq	= XREG_DMAIRQ;

	reg_info.fctl_write	= 0x06;

	set_reg_info(reg_info);
}

//==============================================================================
void CC_253x_254x::supported_units(Unit_ID_List &units)
{
	units.push_back(Unit_ID(0x2530, "CC2530"));
	units.push_back(Unit_ID(0x2531, "CC2531"));
	units.push_back(Unit_ID(0x2533, "CC2533"));
	units.push_back(Unit_ID(0x2540, "CC2540"));
	units.push_back(Unit_ID(0x2541, "CC2541"));
	units.push_back(Unit_ID(0x2543, "CC2543"));
	units.push_back(Unit_ID(0x2544, "CC2544"));
	units.push_back(Unit_ID(0x2545, "CC2545"));
}

//==============================================================================
void CC_253x_254x::find_unit_info(UnitInfo &unit_info)
{
	bool small_unit =
			unit_info.ID == 0x2543 ||
			unit_info.ID == 0x2544 ||
			unit_info.ID == 0x2545;

	unit_info.flags = UnitInfo::SUPPORT_INFO_PAGE;

	ByteVector sfr;
	read_xdata_memory(0x6276, 2, sfr);

	if (sfr[0] & 0x08)
		unit_info.flags |= UnitInfo::SUPPORT_USB;

	unit_info.ram_size = (sfr[1] & 0x07) + 1;

	uint8_t flash_size_id = (sfr[0] >> 4) & 0x07;
	switch (flash_size_id)
	{
	case 0x01:
		unit_info.flash_size = 32;
		break;
	case 0x02:
		unit_info.flash_size = 64;
		break;
	case 0x03:
		unit_info.flash_size = (unit_info.ID == 0x2533) ? 96 : 128;
		break;
	case 0x04:
		unit_info.flash_size = 256;
		break;
	}

	if (small_unit)
	{
		if (flash_size_id == 0x01)
			unit_info.flash_size = 18;
		if (flash_size_id == 0x07)
			unit_info.flash_size = 32;
	}

	unit_info.flash_sizes.push_back(16);//
	unit_info.flash_sizes.push_back(32);
	unit_info.flash_sizes.push_back(64);
	unit_info.flash_sizes.push_back(96);
	unit_info.flash_sizes.push_back(128);
	unit_info.flash_sizes.push_back(256);

	///unit_info.flash_size = 64;////

	if (!small_unit)
	{
		unit_info.flash_page_size = (unit_info.ID == 0x2533) ? 1 : 2;
		unit_info.max_flash_size = (unit_info.ID == 0x2533) ? 96 : 256;

		unit_info.flags |= UnitInfo::SUPPORT_MAC_ADDRESS;

		unit_info.mac_address_count = 2;
		unit_info.mac_address_size =
			(unit_info.ID == 0x2540 || unit_info.ID == 0x2541) ? 6 : 8;
	}
	else
	{
		unit_info.flash_page_size = 1;
		unit_info.max_flash_size = 32;

		UnitCoreInfo reg_info(get_reg_info());

		reg_info.write_block_size	= 512;
		reg_info.verify_block_size	= 512;
		reg_info.dma0_cfg_offset	= 0x0200;
		set_reg_info(reg_info);
	}

	read_xdata_memory(0x6249, 1, sfr);
	unit_info.revision = sfr[0];

	read_xdata_memory(0x624A, 1, sfr);
	unit_info.internal_ID = sfr[0];

	unit_info_ = unit_info;
}

//==============================================================================
void CC_253x_254x::read_info_page(ByteVector &info_page)
{
	log_info("programmer, read info page");

	size_t address = INFO_PAGE_OFFSET;
	size_t total_size = INFO_PAGE_SIZE;
	ByteVector data;

	while (total_size)
	{
		size_t count = std::min(XDATA_READ_CHUNK_SIZE, total_size);
		read_xdata_memory(address, count, data);
		info_page.insert(info_page.end(), data.begin(), data.end());
		total_size -= count;
		address += count;
	}
}

//==============================================================================
void CC_253x_254x::mac_address_read(size_t index, ByteVector &mac_address)
{
	CHECK_PARAM(unit_info_.flags & UnitInfo::SUPPORT_MAC_ADDRESS);

	log_info("programmer, read mac address %u", index);

	if (index == 0)
	{
		size_t offset = (unit_info_.ID == 0x2540 || unit_info_.ID == 0x2541) ?
				0x780E : 0x780C;

		read_xdata_memory(offset, unit_info_.mac_address_size, mac_address);
	}
	if (index == 1)
	{
		size_t offset = unit_info_.flash_size * 1024 - LOCK_DATA_SIZE -
				unit_info_.mac_address_size;

		flash_read_start();
		flash_read_block(offset, unit_info_.mac_address_size, mac_address);
		flash_read_end();
	}
}

//==============================================================================
bool CC_253x_254x::erase_check_comleted()
{
	log_info("programmer, check erase is completed");

	uint8_t status = 0;

	read_debug_status(status);
	return !(status & DEBUG_STATUS_CHIP_ERASE_BUSY);
}

//==============================================================================
void CC_253x_254x::flash_select_bank(uint_t bank)
{
	log_info("programmer, set flash bank %u", bank);

	uint8_t command[] = { 0xBE, 0x57, 0x75, 0x9F, LOBYTE(bank) };

	usb_device_.bulk_write(endpoint_out_, sizeof(command), &command[0]);
}

//==============================================================================
void CC_253x_254x::flash_read_block(size_t offset, size_t size, ByteVector &data)
{
	flash_read(offset, size, data);
}

//==============================================================================
void CC_253x_254x::convert_lock_data(const StringVector& qualifiers,
		ByteVector& lock_data)
{
	ByteVector data(LOCK_DATA_SIZE, 0xFF);

	foreach (const String &s, qualifiers)
	{
		if (s == "debug")
			data[LOCK_DATA_SIZE - 1] &= ~0x80;
		else
		if (s == "pages" || s == "flash")
		{
			uint8_t bit = (data[LOCK_DATA_SIZE - 1] & 0x80); // save debug bit
			std::fill(data.begin(), data.end(), 0xFF);
			data[LOCK_DATA_SIZE - 1] &= (~0x80 | bit); 		 // restore debug bit
		}
		else
		if (s.find("pages:") == 0)
		{
			BoolVector pages;
			read_range(s.substr(ARRAY_SIZE("pages:") - 1), pages,
					0, MAX_PAGE_COUNT - 1);
			uint_t count = std::min(pages.size(), MAX_PAGE_COUNT);
			for (uint_t i = 0; i < count; i++)
				if (pages[i])
					data[i / 8] &= ~(1 << (i % 8));
		}
		else
			throw std::runtime_error("unknown lock qualifyer: " + s);
	}
	lock_data = data;
}

//==============================================================================
bool CC_253x_254x::flash_image_embed_mac_address(DataSectionStore &sections,
		const ByteVector &mac_address)
{
	if (!(unit_info_.flags & UnitInfo::SUPPORT_MAC_ADDRESS))
		return false;

	DataSection section(mac_address_offset(), mac_address);
	sections.add_section(section, true);
	return true;
}

//==============================================================================
bool CC_253x_254x::flash_image_embed_lock_data(DataSectionStore &sections,
		const ByteVector &lock_data)
{
	DataSection section(lock_data_offset(), lock_data);
	sections.add_section(section, true);
	return true;
}

//==============================================================================
uint_t CC_253x_254x::mac_address_offset() const
{
	return unit_info_.flash_size * 1024 - LOCK_DATA_SIZE -
			unit_info_.mac_address_size;
}

//==============================================================================
uint_t CC_253x_254x::lock_data_offset() const
{
	return unit_info_.flash_size * 1024 - LOCK_DATA_SIZE;
}

//==============================================================================
bool CC_253x_254x::config_write(const ByteVector &mac_address,
		const ByteVector &lock_data)
{
	if (mac_address.empty() && lock_data.empty())
		return true;

	const uint_t page_size = unit_info_.flash_page_size * 1024;
	const uint_t page_offset = unit_info_.flash_size * 1024 - page_size;

	ByteVector block;
	flash_read(page_offset, page_size, block);

	if (!empty_block(&block[0], block.size()))
		erase_page(page_offset);

	if (!mac_address.empty())
	{
		CHECK_PARAM(mac_address.size() == unit_info_.mac_address_size);
		CHECK_PARAM(unit_info_.flags & UnitInfo::SUPPORT_MAC_ADDRESS);

		memcpy(&block[mac_address_offset() - page_offset], &mac_address[0],
				unit_info_.mac_address_size);
	}

	if (!lock_data.empty())
	{
		CHECK_PARAM(lock_data.size() == LOCK_DATA_SIZE);

		memcpy(&block[lock_data_offset() - page_offset], &lock_data[0],
				LOCK_DATA_SIZE);
	}

	DataSectionStore store;
	store.add_section(DataSection(page_offset, block), true);
	write_flash_slow(store);

	ByteVector check_block;
	flash_read(page_offset, page_size, check_block);

	return memcmp(&check_block[0], &block[0], block.size()) == 0;
}

//==============================================================================
void CC_253x_254x::flash_write(const DataSectionStore &sections)
{
	if (unit_info_.ID == 0x2543 ||
		unit_info_.ID == 0x2544 ||
		unit_info_.ID == 0x2545)
	{
		write_flash_slow(sections);
		return;
	}

	// Buffers
	const uint16_t ADDR_BUF0     = 0x0000; // 1K
	const uint16_t ADDR_BUF1     = 0x0400; // 1K
	const uint16_t ADDR_DMA_DESC = 0x0800; // 32 bytes

	// DMA Channels
	const uint8_t CH_DBG_TO_BUF0   = 0x02;
	const uint8_t CH_DBG_TO_BUF1   = 0x04;
	const uint8_t CH_BUF0_TO_FLASH = 0x08;
	const uint8_t CH_BUF1_TO_FLASH = 0x10;

	const uint16_t PROG_BLOCK_SIZE = 1024;

	const uint8_t dma_desc[32] = {
		// Debug Interface -> Buffer 0 (Channel 1)
		HIBYTE(XREG_DBGDATA), // src[15:8]
		LOBYTE(XREG_DBGDATA), // src[7:0]
		HIBYTE(ADDR_BUF0), // dest[15:8]
		LOBYTE(ADDR_BUF0), // dest[7:0]
		HIBYTE(PROG_BLOCK_SIZE), LOBYTE(PROG_BLOCK_SIZE), 31, // trigger DBG_BW
		0x11, // increment destination

		// Debug Interface -> Buffer 1 (Channel 2)
		HIBYTE(XREG_DBGDATA), // src[15:8]
		LOBYTE(XREG_DBGDATA), // src[7:0]
		HIBYTE(ADDR_BUF1), // dest[15:8]
		LOBYTE(ADDR_BUF1), // dest[7:0]
		HIBYTE(PROG_BLOCK_SIZE), LOBYTE(PROG_BLOCK_SIZE), 31, // trigger DBG_BW
		0x11, // increment destination

		// Buffer 0 -> Flash controller (Channel 3)
		HIBYTE(ADDR_BUF0), // src[15:8]
		LOBYTE(ADDR_BUF0), // src[7:0]
		HIBYTE(XREG_FWDATA), // dest[15:8]
		LOBYTE(XREG_FWDATA), // dest[7:0]
		HIBYTE(PROG_BLOCK_SIZE), LOBYTE(PROG_BLOCK_SIZE), 18, // trigger FLASH
		0x42, // increment source

		// Buffer 1 -> Flash controller (Channel 4)
		HIBYTE(ADDR_BUF1), // src[15:8]
		LOBYTE(ADDR_BUF1), // src[7:0]
		HIBYTE(XREG_FWDATA), // dest[15:8]
		LOBYTE(XREG_FWDATA), // dest[7:0]
		HIBYTE(PROG_BLOCK_SIZE), LOBYTE(PROG_BLOCK_SIZE), 18, // trigger FLASH
		0x42 // increment source
	};

	ByteVector xsfr;

	// Load dma descriptors
	write_xdata_memory(ADDR_DMA_DESC, dma_desc, sizeof(dma_desc));

	// Set the pointer to the DMA descriptors
	write_xdata_memory(XREG_DMA1CFGL, LOBYTE(ADDR_DMA_DESC));
	write_xdata_memory(XREG_DMA1CFGH, HIBYTE(ADDR_DMA_DESC));

	write_xdata_memory(XREG_FADDRL, 0);
	write_xdata_memory(XREG_FADDRH, 0);

	ByteVector data;
	sections.create_image(FLASH_EMPTY_BYTE, data);
	data.resize(align_up(sections.upper_address(), PROG_BLOCK_SIZE), FLASH_EMPTY_BYTE);

	pw_.write_start(data.size());

	uint8_t dbg_arm, flash_arm;
	for (size_t i = 0; i < (data .size() / PROG_BLOCK_SIZE); i++)
	{
		if ((i & 0x0001) == 0)
		{
			dbg_arm = CH_DBG_TO_BUF0;
			flash_arm = CH_BUF0_TO_FLASH;
		}
		else
		{
			dbg_arm = CH_DBG_TO_BUF1;
			flash_arm = CH_BUF1_TO_FLASH;
		}
		// transfer next buffer (first buffer when i == 0)
		write_xdata_memory(XREG_DMAARM, dbg_arm);

		ByteVector command;
		command.push_back(0xEE);
		command.push_back(0x84);
		command.push_back(0x00);
		command.insert(command.end(), &data[i * PROG_BLOCK_SIZE],
				&data[i * PROG_BLOCK_SIZE] + PROG_BLOCK_SIZE);

		usb_device_.bulk_write(endpoint_out_, command.size(), &command[0]);

		// wait for write to finish
		while (read_xdata_memory(XREG_FCTL) & 0x80);

		write_xdata_memory(XREG_DMAARM, flash_arm);
		write_xdata_memory(XREG_FCTL, 0x06);

		pw_.write_progress(PROG_BLOCK_SIZE);
	}
	while (read_xdata_memory(XREG_FCTL) & FCTL_BUSY); // wait for the last buffer

	pw_.write_finish();
}
