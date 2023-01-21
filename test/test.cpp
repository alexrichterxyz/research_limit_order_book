#include "test.hpp"

named_tests tests::m_tests;

void tests::add(const std::string &name, const test_function &function) {
	m_tests[name] = function;
}

bool tests::run() {
	std::size_t passed_count = 0;

	for (const auto &[name, function] : m_tests) {
		bool passed_test = false;
		try {
			passed_test = function();
		} catch (...) {
			passed_test = false;
		}

		if (passed_test) {
			++passed_count;
		}

		std::cout << std::setfill('.') << std::setw(60) << std::left
			  << name << std::setw(20) << std::right
			  << (passed_test ? "✅" : "❌") << '\n'
			  << std::setfill(' ');
	}

	std::cout << "\nPassed: " << passed_count << '/' << m_tests.size()
		  << '\n';

	return passed_count == m_tests.size();
}

int main() {
	tests::run();
	return 0;
}