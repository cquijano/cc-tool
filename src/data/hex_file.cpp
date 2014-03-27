/*
 * hex_file.cpp
 *
 * Created on: Jul 22, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * License: GNU GPL v2
 *
 */

#include <numeric>
#include <fstream>
#include "file.h"
#include "hex_file.h"

void file_io_error(const String &message, const String &file_name); // throw

class HexDataReader
{
public:
	enum Error
	{
		ERROR_OK,
		ERROR_RECORD_CRC_MISMATCH,
		ERROR_RECORD_NO_HEADER,
		ERROR_RECORD_BAD_FORMAT,
		ERROR_RECORD_UNKNOWN_TYPE,
		ERROR_RECORD_SIZE_MISMATCH,
		ERROR_RECORD_BAD_CHARACTER,
		ERROR_SECTION_OVERLAPPING,
	};

	Error read_next_record(const String &record_line);
	Error read_complete();

	HexDataReader(DataSectionStore &sections, bool ignore_crc);

private:
	DataSectionStore &store_;
	uint_t address_prefix_;
	bool section_started_;
	//Error error_;
	DataSection section_;
	bool ignore_crc_;
};

struct Record
{
	enum Type {
		RT_DATA	= 0,
		RT_EOF = 1,
		RT_EX_SEGMENT_ADDRESS = 2,
		RT_START_SEGMENT_ADDRESS = 3,
		RT_EX_LINEAR_ADDRESS = 4,
		RT_START_LINEAR_ADDRESS = 5,

		MAX_TYPE_NUMBER	= RT_START_LINEAR_ADDRESS,
	};

	Type type;
	uint_t address;
	ByteVector data;
};

static void hex_file_error(
		const String &file_name,
		HexDataReader::Error error,
		uint_t line_number); // throw

//==============================================================================
std::ostream& operator <<(std::ostream &os, const Record &o)
{
	os << "Type: ";

	if (o.type == Record::RT_DATA)
		os << "Data";
	if (o.type == Record::RT_EOF)
		os << "End Of File";
	if (o.type == Record::RT_EX_SEGMENT_ADDRESS)
		os << "Extended Segment Address";
	if (o.type == Record::RT_START_SEGMENT_ADDRESS)
		os << "Start Segment Address";
	if (o.type == Record::RT_EX_LINEAR_ADDRESS)
		os << "Extended Linear Address";
	if (o.type == Record::RT_START_LINEAR_ADDRESS)
		os << "Start Linear Address ";
	os << ", ";

	os << std::uppercase << std::hex << std::setfill('0');
	os << "address: " << std::setw(8) << o.address  << ", ";
	os << "size: " << std::dec << o.data.size();

	return os;
}

//==============================================================================
static bool hex_to_byte(char low_part, char high_part, uint8_t &out_byte)
{
	if (!isxdigit(low_part) || !isxdigit(high_part))
		return false;

	char buffer[3] = { low_part, high_part, 0 };
	out_byte = strtoul(buffer, NULL, 16);
	return true;
}

//==============================================================================
static bool hex_to_binary(const char data[], size_t size, uint8_t out[])
{
	for (size_t i = 0; i < size; i += 2)
	{
		if (!hex_to_byte(data[i], data[i + 1], out[i / 2]))
			return false;
	}
	return true;
}

//==============================================================================
static uint_t make_word(uint8_t low_byte, uint8_t high_byte, bool big_endian = true)
{
	return big_endian ? (low_byte << 8) | high_byte : (high_byte << 8) | low_byte;
}

//==============================================================================
static HexDataReader::Error parse_record(const String &line, Record &record, bool ignore_crc)
{
	if (line[0] != ':')
		return HexDataReader::ERROR_RECORD_NO_HEADER;

	if (!(line.size() % 2) || line.size() > 1024)
		return HexDataReader::ERROR_RECORD_SIZE_MISMATCH;

	size_t record_size = line.size() / 2;
	uint8_t data[record_size];

	if (!hex_to_binary(line.c_str() + 1, line.size() - 1, data))
		return HexDataReader::ERROR_RECORD_BAD_CHARACTER;

	uint8_t crc = 0;
	if (!ignore_crc && std::accumulate(data, data + record_size, crc) != 0)
		return HexDataReader::ERROR_RECORD_CRC_MISMATCH;

	uint8_t data_size = data[0];
	if (data_size != (record_size - 5)) // 5 - header size
		return HexDataReader::ERROR_RECORD_SIZE_MISMATCH;

	uint8_t record_type = data[3];
	if (record_type > Record::MAX_TYPE_NUMBER)
		return HexDataReader::ERROR_RECORD_UNKNOWN_TYPE;

	if (record_type == Record::RT_DATA)
		record.data.assign(data + 4, data + 4 + data_size);
	else
		record.data.clear();

	uint_t address = make_word(data[1], data[2]);

	if (record_type == Record::RT_EX_SEGMENT_ADDRESS)
	{
		if (data_size != 2 || address)
			return HexDataReader::ERROR_RECORD_BAD_FORMAT;

		address = make_word(data[4], data[5]) << 4;
	}

	if (record_type == Record::RT_EX_LINEAR_ADDRESS)
	{
		if (data_size != 2 || address)
			return HexDataReader::ERROR_RECORD_BAD_FORMAT;

		address = make_word(data[4], data[5]) << 16;
	}

	record.type = (Record::Type)record_type;
	record.address = address;

	return HexDataReader::ERROR_OK;
}

//==============================================================================
HexDataReader::HexDataReader(DataSectionStore &store, bool ignore_crc) :
		store_(store),
		address_prefix_(0),
		section_started_(false),
		ignore_crc_(ignore_crc)
{ }

// Do not pass empty lines here!
//==============================================================================
HexDataReader::Error HexDataReader::read_next_record(const String &record_line)
{
	Record record;

	HexDataReader::Error error = parse_record(record_line, record, ignore_crc_);
	if (error != ERROR_OK)
		return error;

	if (record.type == Record::RT_EOF)
		return ERROR_OK;

	if (record.type == Record::RT_DATA)
	{
		record.address |= address_prefix_;
		if (section_started_ && section_.next_address() != record.address)
		{
			if (!store_.add_section(section_, false))
				return ERROR_SECTION_OVERLAPPING;

			section_.data.clear();
			section_started_ = false;
		}

		if (!section_started_)
		{
			section_started_ = true;
			section_.address = record.address;
		}
		section_.data.insert(section_.data.end(), record.data.begin(), record.data.end());
	}
	if (record.type == Record::RT_EX_SEGMENT_ADDRESS)
		address_prefix_ = record.address;

	if (record.type == Record::RT_EX_LINEAR_ADDRESS)
		address_prefix_ = record.address;

	return ERROR_OK;
}

//==============================================================================
HexDataReader::Error HexDataReader::read_complete()
{
	if (section_started_ && !store_.add_section(section_, false))
		return ERROR_SECTION_OVERLAPPING;
	return ERROR_OK;
}

//==============================================================================
void hex_file_load(const String &file_name,
		DataSectionStore &section_store, bool ignore_crc_mismatch)
{
	std::ifstream in(file_name.c_str());
	if (!in)
		throw std::runtime_error("Unable to open file " + file_name);

	HexDataReader::Error error = HexDataReader::ERROR_OK;
	uint_t line_number = 0;

	HexDataReader reader(section_store, ignore_crc_mismatch);
	String record_line;
	while (in)
	{
		line_number++;
		std::getline(in, record_line);

		if (!record_line.empty() && *record_line.rbegin() == '\r')
			record_line.erase(record_line.end() - 1);

		if (record_line.empty())
			continue;

		error = reader.read_next_record(record_line);
		if (error != HexDataReader::ERROR_OK)
			hex_file_error(file_name, error, line_number);
	}

	error = reader.read_complete();
	if (error != HexDataReader::ERROR_OK)
		hex_file_error(file_name, error, line_number);

	//return section_store;
}

//==============================================================================
static void write_eof_record(std::ostream &out)
{
	out << ":00000001FF";
	out << "\r\n";
}

//==============================================================================
static void write_data_record(std::ostream &out, uint_t address,
		const uint8_t data[], size_t size)
{
	const uint8_t buffer[] = { LOBYTE(size), HIBYTE(address), LOBYTE(address), 0x00 };

	uint8_t crc = 0;
	crc = std::accumulate(buffer, buffer + ARRAY_SIZE(buffer), crc);
	crc = std::accumulate(data, data + size, crc);
	crc = 0x100 - crc;

	out << ":";
	out << binary_to_hex(buffer, ARRAY_SIZE(buffer));
	out << binary_to_hex(data, size);
	out << binary_to_hex(&crc, 1);
	out << "\r\n";
}

//==============================================================================
static void write_segment_record(std::ostream &out, uint16_t address)
{
	const uint8_t buffer[] = { 0x02, 0x00, 0x00, 0x02, HIBYTE(address), LOBYTE(address) };

	uint8_t crc = 0;
	crc = 0x100 - std::accumulate(buffer, buffer + ARRAY_SIZE(buffer), crc);

	out << ":";
	out << binary_to_hex(buffer, ARRAY_SIZE(buffer));
	out << binary_to_hex(&crc, 1);
	out << "\r\n";
}

//==============================================================================
void hex_file_save(const String &file_name, const DataSectionStore &section_store)
{
	std::ofstream out(file_name.c_str());
	if (!out)
		throw FileException("Unable to open file" + file_name);

	uint_t offset = 0;
	foreach (const DataSection &section, section_store.sections())
	{
		uint_t address = section.address;
		uint_t size    = section.size();

		while (size)
		{
			if ((address - offset) > 0xFFFF)
			{
				offset = address & ~0x0F;
				write_segment_record(out, offset >> 4);
			}

			const uint_t RECORD_MAX_DATA_SIZE = 16;
			const uint_t record_size = std::min(RECORD_MAX_DATA_SIZE, size);

			write_data_record(out,
					address - offset,
					&section.data[address - section.address],
					record_size);

			address += record_size;
			size -= record_size;
		}
	}
	write_eof_record(out);
}

//==============================================================================
static void hex_file_error(
		const String &file_name,
		HexDataReader::Error error,
		uint_t line_number)
{
	std::stringstream ss;

	ss << "File '" << file_name << "' load error: ";

	switch (error)
	{
	case HexDataReader::ERROR_RECORD_CRC_MISMATCH:
		ss << "CRC mismatch, line: " << line_number;
		break;

	case HexDataReader::ERROR_RECORD_NO_HEADER:
		ss << "Record header not found, line: " << line_number;
		break;

	case HexDataReader::ERROR_RECORD_BAD_FORMAT:
		ss << "Record format error, line: " << line_number;
		break;

	case HexDataReader::ERROR_RECORD_UNKNOWN_TYPE:
		ss << "Unknown record type, line: " << line_number;
		break;

	case HexDataReader::ERROR_RECORD_SIZE_MISMATCH:
		ss << "Record size mismatch, line: " << line_number;
		break;

	case HexDataReader::ERROR_RECORD_BAD_CHARACTER:
		ss << "Unexpected character, line: " << line_number;
		break;

	case HexDataReader::ERROR_SECTION_OVERLAPPING:
		ss << "Sections overlapped";

	case HexDataReader::ERROR_OK:
	default:
		ss << "ok";
	}

	throw FileException(ss.str());
}
