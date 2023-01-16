# An extendable C++ limit order book for research purposes

⚠️ Please note that this project is currently in the development phase and is not yet available for use

## Overview
A matching engine is the software that executes orders in a financial exchange. This project implements a price-time-priority matching engine which is the type most securities markets operate on.

Not only does this matching engine offer a high level of customization, but it also boasts a user-friendly interface, making it ideal for simulation and research purposes. The engine supports a range of order types and features out-of-the-box, including:

- market orders
- limit orders
- all or nothing
- immediate or cancel
- fill or kill
- good til canceled
- stop orders
- trailing stop orders with relative or absolute offset.

Moreover, the engine's flexibility allows for the creation of custom order types that can implement entire trading strategies. This means, for example, that it is possible to design an order type that cancels all limit orders in the order book every time the market price changes and inserts 100 new ones around the BBO while also logging all trades. This is made possible by the engine's "trigger" and "order" interface.

## Implementation

The matching engine is implemented in modern C++ (version 11 or higher) and has no external dependencies. 

### Orders and triggers
The matching engine is based on two main building blocks, "orders" and "triggers", that can be combined and customized to form nearly any conceivable order type.

**Orders** are simply limit orders that are defined by their side (bid/ask), price, quantity, immediate-or-cancel flag, and all-or-nothing flag. Additionally, orders implement five customizable event handlers: on_accepted, on_rejected, on_traded, on_queued, and on_cancelled. 

**Triggers** are defined by a side (bid/ask) and price. Triggers inserted on the bid side get triggered if the market price (price of last trade) reaches or falls below the specified trigger price. Conversely, triggers inserted on the ask side get triggered if the market price reaches or falls below the trigger price. Triggers implement four customizable event handlers: on_accepted, on_queued, on_triggered, and on_cancelled. Triggers are  essential building blocks for stop and trailing stop orders.

### Performance

The code is designed with the following principles in mind:

1. **extendability over performance**: e.g. storing order pointers as opposed to order values in the book increases the chance of cache misses but allows for order customization through runtime-polymorphism
1. **safety over performance**: e.g. using smart pointers in the public interface as opposed to raw pointers prevents illegal memory access
1. **simplicity over performance**: e.g. every order type is elegantly represented as a "trigger" object, "order" object, or combination thereof. This greatly simplifies the implementation of complicated order types such as traling stop orders.

Nevertheless, you can expect the matching engine to handle over a million standard limit/market order executions per second on standard hardware thanks to the low time-complexity of order and trigger operations. However, it's important to note that the use of all-or-nothing orders and trailing stop may decrease its performance significantly. 

## Examples
### Creating, inserting and cancelling an order
Code:

```c++
#include <iostream>
#include <memory>
#include "book.hpp"
#include "order.hpp"

int main(int argc, char *argv[]) {
    auto my_book = elob::book();

    auto my_order = std::make_shared<elob::order>(
        elob::side::bid, // side
        99.99, // price
        100.0, // quantity
        false, // immediate or cancel
        false // all or nothing
    );

    my_book.insert(my_order);

    // formatted output
    std::cout << my_book << std::endl; 

    // cancelling the order removes it from the book in O(1)
    my_order->cancel();

    return 0;
}
```

Output:
```shell
┌─────────────────BIDS─────────────────┬─────────────────ASKS─────────────────┐
│          PRC         QTY     AON QTY │          PRC         QTY     AON QTY │
│        99.99         100           0 │                                      │
```

### Customizing an order to log its execution prices

Code:
```c++
#include "book.hpp"
#include "order.hpp"
#include <iostream>
#include <memory>

class custom_order : virtual public elob::order {
	public:

	custom_order(const elob::side side, const double price, const double quantity)
	    : elob::order(side, price, quantity){};

	protected:

    // event handler that is automatically called 
    // when the order executed against another
	void on_traded(const std::shared_ptr<elob::order> &other_order) override {
		std::cout
            << "Traded with order at price: "
			<< other_order->get_price()
            << std::endl;
	}
};

int main(int argc, char *argv[]) {
	auto my_book = elob::book();

	// insert a couple ask orders for our custom order to execute against
	for (double price = 115.0; price <= 120.0; ++price) {
		my_book.insert(
		    std::make_shared<elob::order>(elob::side::ask, // side
			price, 100.0
        ));
	}

    // output formatted book before execution
	std::cout << my_book << std::endl;

    // create and insert custom order
	my_book.insert(std::make_shared<custom_order>(elob::side::bid, 120.0, 450.0));

    // output formatted book after execution
	std::cout << my_book << std::endl;

	return 0;
}
```

Output:
```
┌─────────────────BIDS─────────────────┬─────────────────ASKS─────────────────┐
│          PRC         QTY     AON QTY │          PRC         QTY     AON QTY │
│                                      │          115         100           0 │
│                                      │          116         100           0 │
│                                      │          117         100           0 │
│                                      │          118         100           0 │
│                                      │          119         100           0 │
│                                      │          120         100           0 │

Traded with order at price: 115
Traded with order at price: 116
Traded with order at price: 117
Traded with order at price: 118
Traded with order at price: 119
┌─────────────────BIDS─────────────────┬─────────────────ASKS─────────────────┐
│          PRC         QTY     AON QTY │          PRC         QTY     AON QTY │
│                                      │          119          50           0 │
│                                      │          120         100           0 │

```



