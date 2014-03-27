/*
 * cc_uint_info.h
 *
 * Created on: Aug 2, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * License: GNU GPL v2
 *
 */

#ifndef _CC_UNIT_INFO_H_
#define _CC_UNIT_INFO_H_

#include "common.h"
#include "data/data_section.h"

struct Unit_ID
{
	uint_t ID;
	String name;

	Unit_ID() : ID(0) { }
	Unit_ID(uint_t ID_, const String &name_) : ID(ID_), name(name_) { }
};

struct UnitInfo
{
	enum Flags {
		SUPPORT_USB = 0x01, 		// remove
		SUPPORT_MAC_ADDRESS = 0x02, // remove, use mac_address_count to checck
		SUPPORT_INFO_PAGE = 0x04
	};

	uint_t ID; 				// ID used by programmer
	String name;			// like 'CCxxxx'
	uint8_t internal_ID; 	// read from chip
	uint8_t revision;		// read from chip
	uint_t flags;

	uint_t flash_size; 		// in KB
	uint_t max_flash_size; 	// in KB
	UintVector flash_sizes;	// list of possible flash sizes, in KB

	uint_t ram_size; 		// in KB
	uint_t mac_address_count;
	uint_t mac_address_size;
	uint_t flash_page_size; // in KB

	size_t actual_flash_size() const;

	UnitInfo();
};

struct UnitCoreInfo
{
	size_t lock_size;		//
	size_t flash_word_size;
	size_t verify_block_size;
	size_t write_block_size;
	uint16_t xbank_offset;
	uint16_t dma0_cfg_offset;
	uint16_t dma_data_offset;

	// Xdata register addresses;
	uint16_t memctr;
	uint16_t fmap;
	uint16_t rndl;
	uint16_t rndh;
	uint16_t fctl;
	uint16_t fwdata;
	uint16_t faddrl;
	uint16_t faddrh;
	uint16_t dma0_cfgl;
	uint16_t dma0_cfgh;
	uint16_t dma_arm;
	uint16_t dma_req;
	uint16_t dma_irq;

	uint8_t fctl_write;
	uint8_t fctl_erase;

	UnitCoreInfo();
};

typedef std::list<Unit_ID> Unit_ID_List;

void check_param(bool assert, const String& module, uint_t line);
#define CHECK_PARAM(x) check_param(x, __FILE__, __LINE__)

#endif // !_CC_UNIT_INFO_H_
