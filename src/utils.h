#pragma once

#include <string>
#include <boost/algorithm/hex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

namespace devtale {
	namespace utils
	{
		std::string dword_to_hex_string(unsigned long value);
		unsigned long hex_string_to_dword(const std::string& str);
		std::string hex_string_to_bytes(const std::string& str);
	};

}