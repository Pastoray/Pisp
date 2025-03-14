#pragma once
#include <iostream>

#ifdef _DEBUG
#define logger std::cout
#else

struct NullStream
{
	template<typename T>
	NullStream& operator<<(const T&) { return *this; }
} null_logger;

#define logger null_logger
#endif

inline void err_exit(const std::string & context, const size_t index, const std::string & msg)
{
	std::cerr << "[ERROR]" << "[CONTEXT:" << context << "]" << "[INDEX:" << index << "]" << " -> " << msg << std::endl;
	exit(EXIT_FAILURE);
}