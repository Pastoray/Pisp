#pragma once
#include <iostream>

#ifdef _DEBUG
#define LOGGER std::cout
#else

class Logger
{
public:
	Logger& operator<<(std::ostream& (*)(std::ostream&));
	Logger& operator<<(std::ios& (*)(std::ios&));

	template <typename T>
	Logger& operator<<(const T&);
};

extern Logger g_logger;

#define LOGGER g_logger
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