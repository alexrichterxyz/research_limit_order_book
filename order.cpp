#include "order.hpp"
#include "book.hpp"
#include <algorithm>

elob::order::order(const elob::side t_side, const double t_price,
    const double t_quantity, const bool t_immediate_or_cancel,
    const bool t_all_or_nothing)
    : m_side(t_side), m_price(t_price), m_quantity(t_quantity),
      m_immediate_or_cancel(t_immediate_or_cancel),
      m_all_or_nothing(t_all_or_nothing) {}

bool elob::order::cancel() {
	if (m_queued) {
		m_limit_it->second.erase(m_order_it);

		if (m_limit_it->second.is_empty()) {
			if (m_side == side::bid) {
				m_book->m_bids.erase(m_limit_it);
			} else {
				m_book->m_asks.erase(m_limit_it);
			}
		}

		m_book = nullptr;

		return true;
	}

	return false;
}

void elob::order::set_all_or_nothing(const bool t_all_or_nothing) {
	if (t_all_or_nothing == m_all_or_nothing) {
		return;
	}

	if (!m_queued) {
		m_all_or_nothing = t_all_or_nothing;
		return;
	}

	auto &limit_obj = m_limit_it->second;
	auto &aon_order_its = limit_obj.m_aon_order_its;

	if (t_all_or_nothing) { // is queued and change from false to true
		// to ensure price-TIME priority, one needs to find the orevious
		// occurence in m_aon_order_its
		limit_obj.m_aon_quantity += m_quantity;
		limit_obj.m_quantity -= m_quantity;

		auto order_it = m_order_it;
		const auto first_order_it = m_limit_it->second.m_orders.begin();

		while (order_it != first_order_it) {
			--order_it;
			if ((*order_it)->m_all_or_nothing) {
				const auto insert_at_it =
				    ++std::find(aon_order_its.begin(),
					aon_order_its.end(), order_it);
				aon_order_its.insert(insert_at_it, m_order_it);
				return;
			}
		}

		m_limit_it->second.m_aon_order_its.push_back(order_it);

	} else { // is queued and change from true to false
		limit_obj.m_aon_quantity -= m_quantity;
		limit_obj.m_quantity += m_quantity;
		aon_order_its.erase(std::find(
		    aon_order_its.begin(), aon_order_its.end(), m_order_it));
	}
}

void elob::order::set_quantity(const double t_quantity) {
	if (t_quantity <= 0) {
		return;
	}

	if (!m_queued) {
		m_quantity = t_quantity;
		return;
	}

	// order is queued

	if (m_all_or_nothing) {

		m_limit_it->second.m_aon_quantity += t_quantity - m_quantity;
		m_quantity = t_quantity;

		if (t_quantity <= m_quantity) { // decrease quantity

			// attempt to execute itself against all orders
			if (m_side == side::bid &&
			    m_book->bid_is_fillable(*m_order_it)) {

				// todo remove from aon queue
				m_book->begin_order_deferral();
				m_book->execute_queued_aon_bid(*m_order_it);
				m_limit_it->second.erase(m_order_it);

				if (m_limit_it->second.is_empty()) {
					m_book->m_bids.erase(m_limit_it);
				}

				m_book->end_order_deferral();

				m_book = nullptr;
			} else if (m_side == side::ask &&
				   m_book->ask_is_fillable(*m_order_it)) {
				m_book->begin_order_deferral();
				m_book->execute_queued_aon_ask(*m_order_it);
				m_limit_it->second.erase(
				    m_order_it); // todo remove from aon queue

				if (m_limit_it->second.is_empty()) {
					m_book->m_asks.erase(m_limit_it);
				}

				m_book->end_order_deferral();
				m_book = nullptr;
			}

		} else { // increase quantity
			// attempt to execute AON orders on other side
			m_book->begin_order_deferral();
			if (m_side == side::bid) {
				m_book->check_ask_aons(m_price);
			} else {
				m_book->check_bid_aons(m_price);
			}
			m_book->end_order_deferral();
		}

	} else {
		m_limit_it->second.m_quantity += t_quantity - m_quantity;
		m_quantity = t_quantity;

		if (t_quantity <= m_quantity) { // decrease quantity
						// nothing happens
		} else {
			// attempt to execute AON orders on other side
			m_book->begin_order_deferral();
			if (m_side == side::bid) {
				m_book->check_ask_aons(m_price);
			} else {
				m_book->check_bid_aons(m_price);
			}
			m_book->end_order_deferral();
		}
	}
}