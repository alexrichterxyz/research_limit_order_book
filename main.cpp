#include "book.hpp"
#include "order.hpp"
#include <iostream>
#include <memory>

class custom_order : virtual public lob::order {
	public:

	custom_order(const lob::side side, const double price, const double quantity)
	    : lob::order(side, price, quantity){};

	protected:

    // event handler that is automatically called 
    // when the order executed against another
	void on_traded(const std::shared_ptr<lob::order> &other_order) override {
		std::cout
            << "Traded with order at price: "
			<< other_order->get_price()
            << std::endl;
	}
};

int main(int argc, char *argv[]) {
	auto my_book = lob::book();

	// insert a couple ask orders for our custom order to execute against
	for (double price = 115.0; price <= 120.0; ++price) {
		my_book.insert(
		    std::make_shared<lob::order>(lob::side::ask, // side
			price, 100.0
        ));
	}

    // output formatted book before execution
	std::cout << my_book << std::endl;

    // create and insert custom order
	my_book.insert(std::make_shared<custom_order>(lob::side::bid, 120.0, 450.0));

    // output formatted book after execution
	std::cout << my_book << std::endl;

	return 0;
}