#include "order_limit.hpp"
#include "order.hpp"
#include <algorithm>

std::list<std::shared_ptr<lob::order>>::iterator lob::order_limit::insert(
    const std::shared_ptr<order> &t_order) {
	m_orders.push_back(t_order);
	const auto order_it = std::prev(m_orders.end());

	if (t_order->m_all_or_nothing) {
		m_aon_quantity += t_order->m_quantity;
		m_aon_order_its.push_back(order_it);
	} else {
		m_quantity += t_order->m_quantity;
	}

	return order_it;
}

void lob::order_limit::erase(
    const std::list<std::shared_ptr<order>>::iterator &t_order_it) {
	auto &order_obj = *t_order_it;

	if (order_obj->m_all_or_nothing) {
		// todo: improve algo to avoid linear time complexity
		m_aon_order_its.erase(std::find(m_aon_order_its.begin(),
		    m_aon_order_its.end(), t_order_it));
		// avoid floating point issues
		m_aon_quantity -= order_obj->m_quantity;
	} else {
		// avoid floating point issues
		m_quantity -= order_obj->m_quantity;
	}

	order_obj->m_queued = false;
	m_orders.erase(t_order_it);
}

double lob::order_limit::simulate_trade(const double t_quantity) const {

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

double lob::order_limit::trade(const std::shared_ptr<order> &t_order) {
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

lob::order_limit::~order_limit() {
	for (auto &order : m_orders) {
		order->m_book = nullptr;
		order->m_queued = false;
	}
}