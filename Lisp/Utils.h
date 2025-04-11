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


void err_exit(const std::string& msg, const std::string& context = "NONE");

struct Instr;
std::string format_instr(const Instr& instr);