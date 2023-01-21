#ifndef TEST_HPP
#define TEST_HPP
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>

using test_function = std::function<bool(void)>;
using named_tests = std::unordered_map<std::string, test_function>;

class tests {
	private:
	static named_tests m_tests;

	public:
	static void add(const std::string &name, const test_function &function);
	static bool run();
};

#endif // #ifndef TEST_HPP
