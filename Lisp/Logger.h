#pragma once
#include <iostream>

#ifdef _DEBUG
#define LOG_DEBUG(msg) std::cout << msg
#else
#define LOG_DEBUG(msg) ((void)0)
#endif