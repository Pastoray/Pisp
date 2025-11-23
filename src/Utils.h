#pragma once
#include <iostream>

#ifdef _DEBUG
#define LOGGER std::cout
#else

class NullBuffer : public std::streambuf
{
public:
	virtual int overflow(int c) { return c; }
};

class NullStream : public std::ostream
{
public:
	NullStream() : std::ostream(new NullBuffer()) {}
	~NullStream()
	{
		delete this->rdbuf();
	}
};

extern NullStream g_null_stream;

#define LOGGER g_null_stream
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