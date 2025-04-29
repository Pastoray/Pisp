#pragma once
#include <iostream>

#ifdef _DEBUG
#define LOGGER std::cout
#else


struct NullStream
{
	template<typename T>
	NullStream& operator<<(const T&) { return *this; }
} NULL_LOGGER;

#define LOGGER NULL_LOGGER
#endif

#define ERR_EXIT(...) \
	err_exit(__FILE__, __LINE__, __func__, __VA_ARGS__);

template<typename ...Args>
void err_exit(const char* file, int line, const char* func, Args&&... args)
{
	std::ostringstream oss;
	(oss << ... << std::forward<Args>(args));

	std::cerr << "[ERROR] -> " << " at " << file
		<< ":" << line
		<< " in " << func << "\n"
		<< oss.str() << std::endl;

	std::exit(EXIT_FAILURE);
}

struct Instr;
std::string format_instr(const Instr& instr);