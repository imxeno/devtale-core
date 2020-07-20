#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace devtale {
	namespace memory
	{
		bool detour(BYTE* old_func, BYTE* new_func, DWORD len);
		bool data_compare(const unsigned char* p_data, const unsigned char* b_mask, const char* sz_mask);
		DWORD find_pattern(const BYTE* b_mask, const char* sz_mask);
	};

}