#pragma once

class ansistring
{
private:
	char* i8_string_;
	size_t i32_length_;

public:
	explicit ansistring(char* i8_string);

	char* get() const;

	size_t length() const;
};
