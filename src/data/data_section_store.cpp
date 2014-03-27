/*
 * data_section_store.cpp
 *
 * Created on: Jul 30, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * License: GNU GPL v2
 *
 */

#include "data_section_store.h"

//==============================================================================
static bool sections_overlapped(
		const DataSection &section1,
		const DataSection &section2)
{
	if (section1.address >= section2.address &&
			section1.address < section2.next_address())
		return true;

	if (section2.address >= section1.address &&
			section2.address < section1.next_address())
		return true;

	return false;
}

//==============================================================================
static bool check_section_fit(
		const DataSectionList &sections,
		const DataSection &section)
{
	foreach (const DataSection &item, sections)
	{
		if (sections_overlapped(item, section))
			return false;
	}
	return true;
}

//==============================================================================
static void add_insert_section(
		DataSectionList &sections,
		const DataSection &section)
{
	DataSectionList::iterator it = sections.begin();
	for ( ; it != sections.end(); it++)
	{
		if (it->address > section.address)
		{
			if (it != sections.begin())
				--it;
			break;
		}
	}
	sections.insert(it, section);
	/// possibly merge
}

//==============================================================================
static void add_replace_section(
		DataSectionList &sections,
		const DataSection &section)
{
	DataSectionList::iterator cur_section = sections.begin();
	for (; cur_section != sections.end(); cur_section++)
	{
		// new section starts at empty space
		if (section.address < cur_section->address)
		{
			cur_section = sections.insert(cur_section, section);
			// normalze
			break;
		}
		// new section starts in another section
		if (cur_section->next_address() > section.address)
		{
			uint_t section_offset = section.address - cur_section->address;

			// simple case: replace a part of existing section,
			// no size change, no normalize
			if (cur_section->next_address() >= section.next_address())
			{
				std::copy(section.data.begin(), section.data.end(),
						cur_section->data.begin() + section_offset);
				return;
			}

			// new section doen't fit current section so resize it and then
			// normalize
			cur_section->data.resize(section.next_address() - cur_section->address);
			std::copy(section.data.begin(), section.data.end(),
					cur_section->data.begin() + section_offset);
			break;
		}
	}

	if (cur_section == sections.end())
	{
		sections.push_back(section);
		/// possibly merge
		return;
	}

	/// normalize sections:
	DataSectionList::iterator it = cur_section;
	it++;
	for (; it != sections.end(); ++it)
	{
		if (it->address >= cur_section->next_address())
			break; // need merge

		/// old section completely overlapped by new one, remove it
		if (it->address >= cur_section->address &&
				it->next_address() <= cur_section->next_address())
		{
			it = sections.erase(it);
			continue;
		}
		/// old section party overlapped by new one, merge them
		if (it->address >= cur_section->next_address())
		{
			cur_section->data.insert(cur_section->data.end(),
							it->data.begin() + cur_section->next_address() - it->address,
							it->data.end());
			it = sections.erase(it);
		}
	}
}

//==============================================================================
const DataSectionList &DataSectionStore::sections() const
{	return sections_; }

//==============================================================================
size_t DataSectionStore::lower_address() const
{	return sections_.empty() ? 0 : sections_.front().address; }

//==============================================================================
size_t DataSectionStore::upper_address() const
{	return sections_.empty() ? 0 : sections_.back().next_address(); }

//==============================================================================
void DataSectionStore::remove_sections()
{	sections_.clear(); }

//==============================================================================
bool DataSectionStore::add_section(const DataSection &section, bool overwrite)
{
	if (section.empty())
		return true;

	if (!overwrite)
	{
		if (!check_section_fit(sections_, section))
			return false;
		add_insert_section(sections_, section);
	}
	else
		add_replace_section(sections_, section);

	return true;
}

//==============================================================================
bool DataSectionStore::add_sections(const DataSectionStore &store, bool overwrite)
{
	if (!overwrite)
	{
		foreach (const DataSection &item, store.sections())
		{
			if (check_section_fit(sections_, item))
				return false;
		}
		foreach (const DataSection &item, store.sections())
			add_insert_section(sections_, item);
	}
	else
		foreach (const DataSection &item, store.sections())
			add_replace_section(sections_, item);
	return true;
}

//==============================================================================
void DataSectionStore::create_image(uint8_t filler, ByteVector &image) const
{
	image.resize(upper_address(), 0);
	std::fill(image.begin(), image.end(), filler);

	foreach (const DataSection &section, sections_)
		std::copy(section.data.begin(), section.data.end(),	&image[section.address]);
}

//==============================================================================
size_t DataSectionStore::actual_size() const
{
	size_t size = 0;

	foreach (const DataSection &section, sections_)
		size += section.data.size();
	return size;
}
