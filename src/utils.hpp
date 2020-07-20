#pragma once

#include <string>
#include <boost/algorithm/hex.hpp>
#include <boost/lexical_cast.hpp>

namespace devtale {
	namespace utils
	{

		std::string dword_to_hex_string(DWORD value)
		{
			return (boost::format("0x%x") % value).str();
		}

		DWORD hex_string_to_dword(const std::string &str)
		{
			return std::stoul(str.find("0x") == 0 ? str.substr(2) : str, 0, 16);
		}
		
		std::string hex_string_to_bytes(const std::string& str)
		{
			return boost::algorithm::unhex(str.find("0x") == 0 ? str.substr(2) : str);
		}
		
	};

}