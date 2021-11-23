#include "utils.h"

namespace devtale {
	namespace utils {

		std::string dword_to_hex_string(unsigned long value)
		{
			return (boost::format("0x%x") % value).str();
		}

		unsigned long hex_string_to_dword(const std::string& str)
		{
			return std::stoul(str.find("0x") == 0 ? str.substr(2) : str, 0, 16);
		}

		std::string hex_string_to_bytes(const std::string& str)
		{
			return boost::algorithm::unhex(str.find("0x") == 0 ? str.substr(2) : str);

		}

	}
}