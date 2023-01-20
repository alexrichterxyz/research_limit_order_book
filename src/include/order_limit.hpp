#ifndef ORDER_LIMIT_HPP
#define ORDER_LIMIT_HPP
#include <list>
#include <memory>

namespace elob {

class order;
class book;

class order_limit {
	private:
	double m_quantity = 0.0;
	double m_aon_quantity = 0.0;
	/* orders are stored in a doubly-linked list to
		allow for O(1) cancellation.*/
	std::list<order_ptr> m_orders;

	/* The field m_aon_order_its stores
	 * iterators to the all-or-nothing orders in m_orders so that
	 * they can be quickly looked up. This is neccessary because
	 * updating order quantities may render some all-or-nothing
	 * orders executable. When all-or-nothing orders are executed or
	 * canceled, their iterators must be deleted from this list.
	 */
	std::list<std::list<order_ptr>::iterator> m_aon_order_its;

	std::list<elob::order_ptr>::iterator insert(c_order_ptr &t_order);

	/**
	 * @brief Simulates the execution of an order with t_quantity
	 * and returns the amount of quantity remaining. This function
	 * is used to test if all-or-nothing orders are fillable.
	 *
	 * @param t_quantity the amount of quantity to be traded.
	 * @return the amount of quantity remaining.
	 */
	double simulate_trade(const double t_quantity) const;

	/**
	 * @brief Execute an inbound order.
	 *
	 * @param t_order, the inbound order
	 * @return the traded quantity.
	 */
	double trade(elob::c_order_ptr &t_order);
	inline bool is_empty() const { return m_orders.empty(); }
	void erase(const std::list<order_ptr>::iterator &t_order_it);

	public:
	/**
	 * @brief Get the non-all-or-none quantity at this price level.
	 * This quantity can be filled partially.
	 *
	 * @return the non-all-or-none quantity at this price level.
	 */
	inline double get_quantity() const;

	/**
	 * @brief Get the all-or-none quantity at this price level.
	 *
	 * @return the all-or-none quantity at this price level
	 */
	inline double get_aon_quantity() const;

	/**
	 * @brief Get the total number of orders (all-or-nothing
	 * included) at this price level.
	 *
	 * @return std::size_t, the number of orders (all-or-nothing
	 * included) at this price level.
	 */
	inline std::size_t get_order_count() const;

	/**
	 * @brief Get an iterator to the first order in the queue.
	 *
	 * @return std::list<elob::order_ptr>::iterator, iterator
	 * to first order in the queue.
	 */
	inline std::list<order_ptr>::iterator orders_begin();

	/**
	 * @brief Get an iterator to the end of the order queue.
	 *
	 * @return std::list<elob::order_ptr>::iterator, iterator
	 * to the end of the order queue.
	 */
	inline std::list<order_ptr>::iterator orders_end();

	/**
	 * @brief Get the number of orders (including all-or-nothing) at
	 * this price level.
	 *
	 * @return the number of orders (including all-or-nothing) at
	 * this price level.
	 */
	inline std::size_t order_count() const;

	/**
	 * @brief Get the number of all-or-nothing-orders at this price
	 * level
	 *
	 * @return the number of all-or-nothing orders at this price
	 * level.
	 */
	inline std::size_t aon_order_count() const;

	friend book;
	friend order;

	~order_limit();
};

} // namespace elob

#include "order.hpp"
#include <algorithm>

std::list<elob::order_ptr>::iterator elob::order_limit::insert(
    elob::c_order_ptr &t_order) {
	m_orders.push_back(t_order);
	const auto order_it = std::prev(m_orders.end());

	if (t_order->m_all_or_nothing) {
		m_aon_quantity += t_order->m_quantity;
		m_aon_order_its.push_back(order_it);
		t_order->m_aon_order_its_it = std::prev(m_aon_order_its.end());
	} else {
		m_quantity += t_order->m_quantity;
	}

	return order_it;
}

void elob::order_limit::erase(
    const std::list<elob::order_ptr>::iterator &t_order_it) {
	auto &order_obj = *t_order_it;

	if (order_obj->m_all_or_nothing) {
		m_aon_order_its.erase(order_obj->m_aon_order_its_it);
		// avoid floating point issues
		m_aon_quantity -= order_obj->m_quantity;
	} else {
		// avoid floating point issues
		m_quantity -= order_obj->m_quantity;
	}

	order_obj->m_queued = false;
	m_orders.erase(t_order_it);
}

double elob::order_limit::simulate_trade(const double t_quantity) const {

	// quick check if the order has a greater quantity than the entire limit
	const double total_quantity = m_quantity + m_aon_quantity;

	if (t_quantity >= total_quantity) {
		return t_quantity - total_quantity;
	}

	// walk through the orders one by one
	double quantity_remaining = t_quantity;
	auto order_it = m_orders.begin();

	for (const auto &order_obj : m_orders) {
		const double order_quantity = order_obj->m_quantity;

		if (quantity_remaining >= order_quantity) {
			quantity_remaining -= order_quantity;
			++order_it;
		} else if (!order_obj->m_all_or_nothing) {
			return 0.0; // consume non-AON order partially
		}
	}

	return quantity_remaining;
}

double elob::order_limit::trade(elob::c_order_ptr &t_order) {
	double traded_quantity = 0.0;
	double quantity_remaining = t_order->m_quantity;
	auto queued_order_it = m_orders.begin();

	while (queued_order_it != m_orders.end()) {
		const auto queued_order = (*queued_order_it);
		const double queued_order_quantity = queued_order->m_quantity;

		if (quantity_remaining >= queued_order_quantity) {
			// incoming order has more or equal quantity
			erase(queued_order_it++);
			traded_quantity += queued_order_quantity;
			quantity_remaining -= queued_order_quantity;
			t_order->m_quantity = quantity_remaining;
			queued_order->m_quantity = 0.0;
			queued_order->on_traded(t_order); // todo
			t_order->on_traded(queued_order); // todo
			queued_order->m_book = nullptr;
		} else if (!queued_order->m_all_or_nothing) {
			/// consume non-AON order partially
			traded_quantity += quantity_remaining;
			queued_order->m_quantity -= quantity_remaining;
			m_quantity -= quantity_remaining;
			quantity_remaining = 0.0;
			t_order->m_quantity = quantity_remaining;
			queued_order->on_traded(t_order); // todo
			t_order->on_traded(queued_order); // todo
			break; // avoid quantity_remaining > 0.0 check in while
			       // loop
		} else {
			// cannot fill AON orders partially
			++queued_order_it;
		}
	}

	return traded_quantity;
}

double elob::order_limit::get_quantity() const { return m_quantity; }

double elob::order_limit::get_aon_quantity() const { return m_aon_quantity; }

std::size_t elob::order_limit::get_order_count() const {
	return m_orders.size();
}

std::list<elob::order_ptr>::iterator elob::order_limit::orders_begin() {
	return m_orders.begin();
}

std::list<elob::order_ptr>::iterator elob::order_limit::orders_end() {
	return m_orders.end();
}

std::size_t elob::order_limit::order_count() const { return m_orders.size(); }

std::size_t elob::order_limit::aon_order_count() const {
	return m_aon_order_its.size();
}

elob::order_limit::~order_limit() {
	for (auto &order : m_orders) {
		order->m_book = nullptr;
		order->m_queued = false;
	}
}

#endif // #ifndef ORDER_LIMIT_HPP