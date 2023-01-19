#ifndef ORDER_HPP
#define ORDER_HPP
#include <list>
#include <map>
#include <memory>

namespace elob {

class book;

/**
 * @brief the order class defines the fundamental properties of orders
 * including side, price, quantity, and whether the order is immediate
 * or cancel, or all or nothing. Market orders are represented as limit
 * orders with a price of zero (sell) or infinity (buy). Objects of this
 * class can be inserted into book objects. The behavior of orders can
 * be customized by overriding the virtual event methods: on_queued,
 * on_accepted, on_rejected, on_traded, and on_canceled.
 *
 */
class order : public std::enable_shared_from_this<order> {
	private:
	const side m_side;
	const double m_price;
	double m_quantity = 0.0;
	const bool m_immediate_or_cancel = false;
	bool m_all_or_nothing = false;
	bool m_queued = false;

	/* pointer to the book into which the order was inserted.
		it's guaranteed to be dereferencable in the virtual
	   event methods. */
	book *m_book = nullptr;

	/* these iterators store the location of the order in the order
		book. They are used to cancel the order in O(1). */
	std::map<double, elob::order_limit>::iterator m_limit_it;
	std::list<order_ptr>::iterator m_order_it;

	protected:
	/**
	 * @brief book. At this stage the order has been verified to be
	 * valid and is awaiting execution. This event method can be
	 * used to adapt the price of the order to the current market
	 * price of the book.
	 *
	 */
	virtual void on_accepted(){};

	/**
	 * @brief called once the order has been queued at the specified
	 * price level.
	 *
	 */
	virtual void on_queued(){};

	/**
	 * @brief called if the order was rejected by the book. This may
	 * happen if the order is already queued, has a negative price,
	 * etc.
	 *
	 */
	virtual void on_rejected(){};

	/**
	 * @brief called after the order executed against another one.
	 *
	 * @param t_order the other order
	 */
	virtual void on_traded(c_order_ptr &t_order){};

	/**
	 * @brief called once the order got canceled. This may happen if
	 * the order got canceled manually or if the order is immediate
	 * or cancel and could not get filled immediately.
	 *
	 */
	virtual void on_canceled(){};

	public:
	/**
	 * @brief Construct a new order object
	 *
	 * @param t_side the side at which the order will be inserted
	 * (either elob::side::bid or elob::side::ask)
	 * @param t_price the price at which the order will be inserted.
	 * For market orders this will be 0.0 (sell) or DBL_MAX (buy)
	 * @param t_quantity the quantity demanded or provided.
	 * @param t_immediate_or_cancel indicator of whether the order
	 * is immediate or cancel.
	 * @param t_all_or_nothing indicator of whether the order is all
	 * or nothing.
	 */
	order(const side t_side, const double t_price, const double t_quantity,
	    const bool t_immediate_or_cancel = false,
	    const bool t_all_or_nothing = false);

	/**
	 * @brief Cancels the order, if possible. Currently, only queued
	 * orders can be canceled.
	 *
	 * @return true successfully cancelled.
	 * @return false could not cancel the order because it hasn't
	 * been queued yet.
	 */
	inline bool cancel();

	/**
	 * @brief Get the instance of the book into which the order was
	 * inserted. This value is guaranteed to be non-nullptr in the
	 * virtual event methods.
	 *
	 * @return book* pointer to the book object into which the order
	 * was inserted or nullptr if it hasn't been inserted into a
	 * book yet or got removed from it.
	 */
	inline book *get_book() const;

	/**
	 * @brief Get the side of the order.
	 *
	 * @return either elob::side::bid or elob::side::ask.
	 */
	inline side get_side() const;

	/**
	 * @brief Get the price of the order.
	 *
	 * @return double price of the order.
	 */
	inline double get_price() const;

	/**
	 * @brief Get the quantity of the order.
	 *
	 * @return the quantity of the order.
	 */
	inline double get_quantity() const;

	/**
	 * @brief Update the quantity of the order. This operation is
	 * O(1) in some cases but can be very inefficient if there are
	 * lot of all or nothing orders in the book.
	 *
	 * @param t_quantity
	 */
	inline void set_quantity(const double t_quantity);

	/**
	 * @brief Check if the order is immediate or cancel.
	 *
	 * @return true, is immediate or cancel.
	 * @return false, is not immediate or cancel.
	 */
	inline bool is_immediate_or_cancel() const;

	/**
	 * @brief Check if the order is all or nothing.
	 *
	 * @return true, is all or nothing.
	 * @return false, is not all or nothing.
	 */
	inline bool is_all_or_nothing() const;

	/**
	 * @brief Update the order's all or nothing flag.
	 *
	 * @param t_all_or_nothing the update value of the order's all
	 * or nothing flag
	 */
	inline void set_all_or_nothing(const bool t_all_or_nothing);

	/**
	 * @brief Check whether the order is queued. Queued orders can
	 * be canceled.
	 *
	 * @return true, the order is queued.
	 * @return false, the order is not queued.
	 */
	inline bool is_queued() const;

	friend book;
	friend order_limit;
};

} // namespace elob

#include "book.hpp"
#include "common.hpp"
#include "order_limit.hpp"
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

elob::book *elob::order::get_book() const { return m_book; }

elob::side elob::order::get_side() const { return m_side; }

double elob::order::get_price() const { return m_price; }

double elob::order::get_quantity() const { return m_quantity; }

bool elob::order::is_immediate_or_cancel() const {
	return m_immediate_or_cancel;
}

bool elob::order::is_all_or_nothing() const { return m_all_or_nothing; }

bool elob::order::is_queued() const { return m_queued; }

#endif // #ifndef ORDER_HPP