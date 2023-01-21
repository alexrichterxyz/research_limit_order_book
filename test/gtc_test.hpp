#ifndef GTC_TEST_HPP
#define GTC_TEST_HPP
#include "test.hpp"

class gtc_test : public test {
	inline static bool queue_bids();
	inline static bool queue_asks();
	inline static bool insert_marketable_bids();
	inline static bool insert_marketable_asks();

	public:
	gtc_test();
};

#include "../include/book.hpp"
#include "gtc_test.hpp"

gtc_test::gtc_test() : test("gtc_test") {
	add("queue_bids", queue_bids);
	add("queue_asks", queue_asks);
	add("insert_marketable_bids", insert_marketable_bids);
	add("insert_marketable_asks", insert_marketable_asks);
}

bool gtc_test::queue_bids() { return false; }
bool gtc_test::queue_asks() { return false; }
bool gtc_test::insert_marketable_bids() { throw "Error"; }
bool gtc_test::insert_marketable_asks() { return true; }

gtc_test *gtc_test_obj = new gtc_test();

#endif // #ifndef GTC_TEST_HPP