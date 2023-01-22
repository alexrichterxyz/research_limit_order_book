# An extendable C++ limit order book for research purposes 
![Under development, do not use!](https://img.shields.io/badge/⚠%EF%B8%8F-Under%20development%2C%20do%20not%20use!-critical?style=for-the-badge)

Feel free to reach out to me if you are not a bot

📧 alexander.richter @ sydney.edu.au

## Overview
A matching engine is the software that executes orders on a financial exchange. This project contains a price-time-priority matching engine -the type most securities markets operate on. 

This C++ implementation boasts a user friendly, efficient, yet highly customizable interface making it ideal for simulation and research purposes. A wide range of of order types and features are supported out-of-the-box, including:

- market orders
- limit orders
- all or nothing
- immediate or cancel
- fill or kill
- good til canceled
- stop orders
- trailing stop orders with relative or absolute offset.

## Implementation

The matching engine is implemented in modern C++ (version 14 or higher) and has no external dependencies. 

### Orders and triggers
There are two main building blocks: "orders" and "triggers", that can be combined and customized to form nearly any conceivable order type.

**Orders** are simply limit orders that are defined by their side (bid/ask), price, quantity, immediate-or-cancel flag, and all-or-nothing flag. Additionally, orders implement five customizable event handlers: on_accepted, on_rejected, on_traded, on_queued, and on_cancelled. 

**Triggers** are defined by a side (bid/ask) and price. Triggers inserted on the bid side get triggered if the market price (price of last trade) reaches or falls below the specified trigger price. Conversely, triggers inserted on the ask side get triggered if the market price reaches or falls below the trigger price. Triggers implement four customizable event handlers: on_accepted, on_queued, on_triggered, and on_cancelled. Triggers are  essential building blocks for stop and trailing stop orders.

### Performance

The code is designed with the following principles in mind:

1. **extendability over performance**: e.g. storing order pointers as opposed to order values in the book increases the chance of cache misses but allows for order customization through runtime-polymorphism
1. **safety over performance**: e.g. using smart pointers in the public interface as opposed to raw pointers prevents illegal memory access
1. **simplicity over performance**: e.g. every order type is elegantly represented as a "trigger" object, "order" object, or combination thereof. This greatly simplifies the implementation of complicated order types such as traling stop orders.

Nevertheless, you can expect the matching engine to handle over a million standard limit/market order executions per second on standard hardware thanks to the low time-complexity of order and trigger operations. However, it's important to note that the use of all-or-nothing orders and trailing stop may decrease its performance significantly. 