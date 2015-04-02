/*
 * cc_programmer.cpp
 *
 * Created on: Aug 4, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * 	License: GNU GPL v2
 *
 */

#include <stdio.h>
#include "cc_programmer.h"
#include "cc_251x_111x.h"
#include "cc_253x_254x.h"
#include "cc_243x.h"
#include "log.h"

const uint_t DEFAULT_TIMEOUT = 3000;
const uint_t MAX_ERASE_TIME	= 8000;

const static USB_DeviceID DeviceTable[] = {
	{ 0x0451, 0x16A2, 0x84, 0x04, "CC Debugger",
			USB_DeviceID::PROTOCOL_TI },
	{ 0x11A0, 0xDB20, 0x84, 0x04, "SmartRF04 Evaluation Board",
			USB_DeviceID::PROTOCOL_TI },
	{ 0x11A0, 0xEB20, 0x82, 0x02, "SmartRF04 Evaluation Board (Chinese)",
			USB_DeviceID::PROTOCOL_CHIPCON },
	{ 0x0451, 0x16A0, 0x84, 0x04, "SmartRF05 Evaluation Board",
			USB_DeviceID::PROTOCOL_TI },
};

//==============================================================================
static void log_add_target_info(const UnitInfo &unit_info)
{
	log_info("target, name: %s, chip ID: %02Xh, rev. %02Xh, flash: %u, ram: %u, flags: %02Xh",
			unit_info.name.c_str(),
			unit_info.internal_ID,
			unit_info.revision,
			unit_info.flash_size,
			unit_info.ram_size,
			unit_info.flags);
}

//==============================================================================
static void log_add_programmer_info(const CC_ProgrammerInfo &info)
{
	log_info("device, name: %s, ID: %s, version: %04Xh, revision: %04Xh",
			info.name.c_str(),
			info.debugger_id.c_str(),
			info.fw_version,
			info.fw_revision);
}

//==============================================================================
CC_Programmer::CC_Programmer()
{
	usb_device_.set_transfer_timeout(DEFAULT_TIMEOUT);

	unit_drviers_.push_back(CC_UnitDriverPtr(new CC_253x_254x(usb_device_, pw_)));
	unit_drviers_.push_back(CC_UnitDriverPtr(new CC_251x_111x(usb_device_, pw_)));
	unit_drviers_.push_back(CC_UnitDriverPtr(new CC_243x(usb_device_, pw_)));
}

//==============================================================================
USB_DeviceIDVector CC_Programmer::supported_devices() const
{
	USB_DeviceIDVector result;
	foreach (const USB_DeviceID &item, DeviceTable)
		result.push_back(item);

	return result;
}

//==============================================================================
StringVector CC_Programmer::supported_unit_names() const
{
	StringVector names;
	foreach (const CC_UnitDriverPtr &item, unit_drviers_)
	{
		Unit_ID_List units;
		item->supported_units(units);

		foreach (Unit_ID &unit, units)
			names.push_back(unit.name);
	}
	return names;
}

//==============================================================================
bool CC_Programmer::set_debug_interface_speed(CC_Programmer::InterfaceSpeed speed)
{
	log_info("programmer, set debug interface speed %u", speed);

	if (!opened())
		return false;

	const uint8_t USB_SET_DEBUG_INTERFACE_SPEED	= 0xCF;
	uint16_t value = (speed == CC_Programmer::IS_FAST) ? 0 : 1;

	usb_device_.control_write(LIBUSB_REQUEST_TYPE_VENDOR,
			USB_SET_DEBUG_INTERFACE_SPEED,
			value, 0, NULL, 0);

	return true;
}

//==============================================================================
bool CC_Programmer::unit_set_flash_size(uint_t flash_size)
{
	return driver_->set_flash_size(flash_size);
}

//==============================================================================
bool CC_Programmer::programmer_info(CC_ProgrammerInfo &info)
{
	if (opened())
	{
		info = programmer_info_;
		return true;
	}
	return false;
}

//==============================================================================
void CC_Programmer::init_device()
{
	if (programmer_info_.usb_device.protocol == USB_DeviceID::PROTOCOL_CHIPCON)
		usb_device_.reset_device();

	usb_device_.set_configuration(1);
	usb_device_.claim_interface(0);
	request_device_info();
}

//==============================================================================
CC_Programmer::OpenResult CC_Programmer::open(uint_t bus, uint_t device)
{
	if (!usb_device_.open_by_address(bus, device))
		return OR_NOT_FOUND;

	libusb_device_descriptor descriptor;
	usb_device_.device_decriptor(descriptor);

	foreach (const USB_DeviceID &item, DeviceTable)
	{
		if (descriptor.idProduct == item.product_id &&
				descriptor.idVendor == item.vendor_id)
		{
			programmer_info_.usb_device = item;
			init_device();
			return OR_OK;
		}
	}
	return OR_NOT_SUPPORTED;
}

//==============================================================================
CC_Programmer::OpenResult CC_Programmer::open()
{
	foreach (const USB_DeviceID &item, DeviceTable)
	{
		if (usb_device_.open_by_vid_pid(item.vendor_id, item.product_id))
		{
			programmer_info_.usb_device = item;
			break;
		}
	}

	if (!usb_device_.opened())
		return OR_NOT_FOUND;
	init_device();
	return OR_OK;
}

//==============================================================================
void CC_Programmer::close()
{	usb_device_.close(); }

//==============================================================================
bool CC_Programmer::opened() const
{	return usb_device_.opened(); }

//==============================================================================
void CC_Programmer::request_device_info()
{
	libusb_device_descriptor device_descriptor;
	usb_device_.device_decriptor(device_descriptor);

	char DID[16];
	sprintf(DID, "%02X%02X",
			device_descriptor.bcdDevice >> 8,
			device_descriptor.bcdDevice & 0xFF);
	programmer_info_.debugger_id = DID;

	if (device_descriptor.iProduct > 0)
		usb_device_.string_descriptor_ascii(device_descriptor.iProduct, programmer_info_.name);
	else
	{
		foreach (const USB_DeviceID &item, DeviceTable)
		{
			if (device_descriptor.idProduct == item.product_id &&
					device_descriptor.idVendor == item.vendor_id)
			{
				programmer_info_.name = item.description;
				break;
			}
		}
	}

	const uint8_t USB_REQUEST_GET_STATE	= 0xC0;

	log_info("programmer, request device state");

	uint8_t info[8];
	usb_device_.control_read(LIBUSB_REQUEST_TYPE_VENDOR, USB_REQUEST_GET_STATE,
			0, 0, info, 8);

	programmer_info_.fw_version	= info[2] | info[3] << 8;
	programmer_info_.fw_revision = info[4] | info[5] << 8;

	unit_info_.ID = info[0] | info[1] << 8;
	foreach (CC_UnitDriverPtr &driver, unit_drviers_)
	{
		Unit_ID_List units;
		driver->supported_units(units);
		foreach (Unit_ID &unit, units)
		{
			if (unit.ID == unit_info_.ID)
			{
				unit_info_.name = unit.name;
				driver_ = driver;
				break;
			}
		}
	}

	if (!driver_ && unit_info_.ID > 0)
	{
		std::swap(info[0], info[1]);
		unit_info_.name = "CC" + binary_to_hex(info, 2, "");
	}

	if (driver_)
		driver_->set_programmer_ID(programmer_info_.usb_device);

	log_add_programmer_info(programmer_info_);
}

//==============================================================================
void CC_Programmer::unit_status(String &name, bool &supported) const
{
	name = unit_info_.name;
	supported = (driver_ != NULL);
}

//==============================================================================
void CC_Programmer::unit_close()
{	driver_->reset(false); }

//==============================================================================
void CC_Programmer::enter_debug_mode()
{
	log_info("programmer, enter debug mode");

	const uint8_t USB_PREPARE_DEBUG_MODE = 0xC5;

	usb_device_.control_write(LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
			USB_PREPARE_DEBUG_MODE, 0, 0, NULL, 0);

	const uint8_t USB_SET_CHIP_INFO = 0xC8;
	const size_t command_size =
			programmer_info_.usb_device.protocol == USB_DeviceID::PROTOCOL_TI ?
			0x30 : 0x20;

	ByteVector command(command_size, ' ');
	memcpy(&command[0x00], unit_info_.name.c_str(), unit_info_.name.size());
	memcpy(&command[0x10], "DID:", 4);
	memcpy(&command[0x15], programmer_info_.debugger_id.c_str(), programmer_info_.debugger_id.size());

	usb_device_.control_write(LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
			USB_SET_CHIP_INFO, 1, 0, &command[0], command.size());
}

//==============================================================================
bool CC_Programmer::unit_locked()
{
	log_info("programmer, check if unit locked");

	uint8_t status = 0;
	driver_->read_debug_status(status);
	return (status & DEBUG_STATUS_DEBUG_LOCKED);
}

//==============================================================================
bool CC_Programmer::unit_connect(UnitInfo &info)
{
	log_info("programmer, connect target");

	enter_debug_mode();
	driver_->reset(true);

	uint8_t status = 0;
	driver_->read_debug_status(status);

	if ((status & DEBUG_STATUS_DEBUG_LOCKED))
	{
		log_info("programmer, target is locked");
		return true;
	}

	if (!(status & DEBUG_STATUS_CPU_HALTED))
	{
		log_info("programmer, halt failed");
		return false;
	}

	driver_->write_debug_config(DEBUG_CONFIG_TIMER_SUSPEND | DEBUG_CONFIG_SOFT_POWER_MODE);
	driver_->find_unit_info(unit_info_);

	log_add_target_info(unit_info_);

	info = unit_info_;
	return true;
}

//==============================================================================
bool CC_Programmer::unit_reset()
{
	driver_->reset(false);
	return true;
}

//==============================================================================
bool CC_Programmer::unit_erase()
{
	driver_->erase();

	uint_t erase_timer = get_tick_count();
	do
	{
		if (driver_->erase_check_comleted())
			return true;
		usleep(500 * 1000);
	}
	while ((get_tick_count() - erase_timer) <= MAX_ERASE_TIME);

	return false; // erase timeout
}

//==============================================================================
void CC_Programmer::unit_read_info_page(ByteVector &info_page)
{
	pw_.enable(false);
	driver_->read_info_page(info_page);
}

//==============================================================================
void CC_Programmer::unit_mac_address_read(size_t index, ByteVector &mac_address)
{
	pw_.enable(false);
	driver_->mac_address_read(index, mac_address);
}

//==============================================================================
bool CC_Programmer::unit_config_write(ByteVector &mac_address, ByteVector &lock_data)
{
	pw_.enable(false);
	return driver_->config_write(mac_address, lock_data);
}

//==============================================================================
bool CC_Programmer::flash_image_embed_mac_address(DataSectionStore &sections,
		const ByteVector &mac_address)
{
	return driver_->flash_image_embed_mac_address(sections, mac_address);
}

//==============================================================================
bool CC_Programmer::flash_image_embed_lock_data(DataSectionStore &sections,
		const ByteVector &lock_data)
{
	return driver_->flash_image_embed_lock_data(sections, lock_data);
}

//==============================================================================
uint_t CC_Programmer::unit_lock_data_size() const
{
	return driver_->lock_data_size();
}

//==============================================================================
void CC_Programmer::unit_convert_lock_data(const StringVector& qualifiers,
		ByteVector& lock_data)
{
	driver_->convert_lock_data(qualifiers, lock_data);
}

//==============================================================================
void CC_Programmer::unit_flash_read(ByteVector &flash_data)
{
	pw_.enable(true);
	pw_.read_start(unit_info_.actual_flash_size());

	driver_->flash_read_start();
	driver_->flash_read_block(0, unit_info_.actual_flash_size(), flash_data);
	driver_->flash_read_end();

	pw_.read_finish();
}

//==============================================================================
bool CC_Programmer::unit_flash_verify(const DataSectionStore &sections,
		CC_Programmer::VerifyMethod method)
{
	log_info("programmer, verify flash, %u", method);

	pw_.enable(true);

	if (method == VM_BY_CRC)
		return driver_->flash_verify_by_crc(sections);
	return driver_->flash_verify_by_read(sections);
}

//==============================================================================
void CC_Programmer::unit_flash_write(const DataSectionStore &sections)
{
	pw_.enable(true);
	driver_->flash_write(sections);
}

//==============================================================================
void CC_Programmer::do_on_flash_read_progress(
		const ProgressWatcher::OnProgress::slot_type &slot)
{	pw_.do_on_read_progress(slot); }

//==============================================================================
void CC_Programmer::do_on_flash_write_progress(
		const ProgressWatcher::OnProgress::slot_type &slot)
{	pw_.do_on_write_progress(slot); }
