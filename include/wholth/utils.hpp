#ifndef WHOLTH_UTILS_H_
#define WHOLTH_UTILS_H_

#include <iostream>

#define LOG_RED(x) std::cout << "\033[1;31m" << x << "\033[0m\n"
#define LOG_GREEN(x) std::cout << "\033[1;32m" << x << "\033[0m\n"
#define LOG_YELLOW(x) std::cout << "\033[1;33m" << x << "\033[0m\n"
#define LOG_BLUE(x) std::cout << "\033[1;34m" << x << "\033[0m\n"
#define LOG_MAGENTA(x) std::cout << "\033[1;35m" << x << "\033[0m\n"
#define LOG_CYAN(x) std::cout << "\033[1;36m" << x << "\033[0m\n"
#define LOG_WHITE(x) std::cout << "\033[1;37m" << x << "\033[0m\n"
#define LOG(x) std::cout << x << '\n'

#endif // WHOLTH_UTILS_H_
