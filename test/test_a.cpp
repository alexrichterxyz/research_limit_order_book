#include "test_a.hpp"

bool test_a::some_test() { return false; }

test_a::test_a() { add("some_test", some_test); }

test_a test_a_obj;