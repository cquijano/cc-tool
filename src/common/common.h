/*
 * common.h
 *
 * Created on: Aug 3, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * License: GNU GPL v2
 *
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <algorithm>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

#define foreach 					BOOST_FOREACH
#define ARRAY_SIZE(a) 				(sizeof(a) / sizeof(a[0]))
#define ALIGN_UP(value, aligment) 	(((value + (aligment) - 1) / (aligment)) * (aligment))
#define LOBYTE(w) 					((uint8_t)(w))
#define HIBYTE(w) 					((uint8_t)(((uint16_t)(w) >> 8) & 0xFF))

typedef unsigned int uint_t;
typedef std::string 			String;
typedef std::vector<String> 	StringVector;
typedef std::vector<uint8_t> 	ByteVector;
typedef std::vector<bool> 		BoolVector;
typedef std::vector<uint_t> 	UintVector;

String 	convinient_storage_size(off_t size);
uint_t 	get_tick_count();
String 	binary_to_hex(const uint8_t data[], size_t size, const char delimiter[] = "");
String 	binary_to_hex(const ByteVector &data, const char delimiter[] = "");
void 	vector_append(ByteVector &vector, const uint8_t data[], size_t size);
String& string_append(String &string, const String &item, const char *delimiter);
bool 	hex_to_binary(const String &hex, ByteVector &out, const char delimiter[] = "");
uint_t	align_up(uint_t value, uint_t aligment);

namespace common_impl
{
	template<typename T>
	struct nt { typedef T type; };
	template<>
	struct nt<unsigned char> { typedef uint_t type; };
	template<>
	struct nt<char> { typedef int type; };
};

//==============================================================================
template<typename T>
inline String number_to_string(T number)
{
	std::stringstream ss;
	ss << static_cast<typename common_impl::nt<T>::type>(number);
	return ss.str();
}

//==============================================================================
template<typename T>
inline bool string_to_number(const String &string, T &number)
{
	char *bad_character = NULL;
	number = strtoul(string.c_str(), &bad_character, 10);

	return string.empty() || *bad_character == '\0';
}

#endif // !_COMMON_H_
