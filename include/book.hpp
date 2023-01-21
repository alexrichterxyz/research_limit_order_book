#ifndef BOOK_HPP
#define BOOK_HPP
#include "common.hpp"
#include <map>
#include <memory>
#include <queue>

namespace elob {

class book;
class order_limit;
class trigger_limit;
class trigger;
class order;

std::ostream &operator<<(std::ostream &t_os, const book &t_book);

/**
 * @brief book implements a price-time-priotity matching engine. Orders
 * and triggers can be inserted into book objects.
 *
 */
class book {
	private:
	/* During order execution. event handlers like "on_trade" are
	 * called which may insert additional orders recursively. These
	 * additional orders will be deferred. Only once
	 * the outer insertion call has completed, the additional orders
	 * are removed from the deferral queue and executed. */
	std::size_t m_order_deferral_depth = 0;
	std::queue<order_ptr> m_deferred;

	std::map<double, order_limit, std::greater<double>> m_bids;
	std::map<double, order_limit, std::less<double>> m_asks;

	std::map<double, trigger_limit, std::greater<double>> m_bid_triggers;
	std::map<double, trigger_limit, std::less<double>> m_ask_triggers;

	// set to -1 to prevent triggers from being triggered
	// immediately.
	double m_market_price = -1.0;

	/**
	 * \internal
	 * @brief When called, subsequent orders will be deferred rather
	 * than being queued immediately. This is required to ensure
	 * orders are fully executed before new orders, e.g. those
	 * inserted from within event handlers, are executed.
	 *
	 */
	inline void begin_order_deferral();

	/**
	 * \internal
	 * @brief Once the outer insertion call has been completed,
	 * orders from the deferral queue are executed.
	 *
	 */
	inline void end_order_deferral();

	inline void insert_bid(c_order_ptr &t_order);
	inline void insert_ask(c_order_ptr &t_order);

	inline void insert_aon_bid(c_order_ptr &t_order);
	inline void insert_aon_ask(c_order_ptr &t_order);

	/**
	 * \internal
	 * @brief Check if the bid order can be filled completely. This
	 * check is performed before all-or-nothing orders are executed.
	 *
	 * @param t_order the (all-or-nothing) bid order to be executed.
	 * @return true the order is completely fillable
	 * @return false the order is only partially fillable
	 */
	inline bool bid_is_fillable(c_order_ptr &t_order) const;

	/**
	 * \internal
	 * @brief Check if the ask order can be filled completely. This
	 * check is performed before all-or-nothing orders are executed.
	 *
	 * @param t_order the (all-or-nothing) ask order to be executed.
	 * @return true the order is completely fillable
	 * @return false the order is only partially fillable
	 */
	inline bool ask_is_fillable(c_order_ptr &t_order) const;

	inline void execute_bid(c_order_ptr &t_order);
	inline void execute_ask(c_order_ptr &t_order);

	inline void execute_queued_aon_bid(c_order_ptr &t_order);
	inline void execute_queued_aon_ask(c_order_ptr &t_order);

	inline void queue_bid_order(c_order_ptr &t_order);
	inline void queue_ask_order(c_order_ptr &t_order);

	inline void queue_bid_trigger(c_trigger_ptr &t_trigger);
	inline void queue_ask_trigger(c_trigger_ptr &t_trigger);

	/**
	 * @brief Check if any all-or-nothing bids at the specified
	 * price or lower are executable. This function is called if the
	 * quantity of queued orders is increased.
	 *
	 * @param t_price the price from which queued all-or-nothing
	 * will be checked.
	 */
	inline void check_bid_aons(const double t_price);

	/**
	 * @brief Check if any all-or-nothing asks at the specified
	 * price or higher are executable. This function is called if
	 * the quantity of queued orders is increased.
	 *
	 * @param t_price the price from which queued all-or-nothing
	 * will be checked.
	 */
	inline void check_ask_aons(const double t_price);

	public:
	/**
	 * @brief Inserts an order into the book. Marketable orders will
	 * be executed. Partially filled orders will be queued (or
	 * canelled if marked as immediate-or-cancel). When the function
	 * is called from within another order's event handler (like
	 * on_trade), the order will be deferred and only executed once
	 * the other order has been handled.
	 *
	 * @param t_order the order to be inserted
	 */
	inline void insert(c_order_ptr t_order);

	/**
	 * @brief Inserts a trigger into the book. Unlike orders,
	 * triggers will cannot be deferred and will instead be queued
	 * immediately.
	 *
	 * @param t_trigger the trigger to be inserted
	 */
	inline void insert(c_trigger_ptr t_trigger);

	/**
	 * @brief Get the best bid price.
	 *
	 * @return double the best bid price
	 */
	inline double get_bid_price() const;

	/**
	 * @brief Get the best ask price.
	 *
	 * @return double the best ask price
	 */
	inline double get_ask_price() const;

	/**
	 * @brief Get the price at which the last trade occured.
	 *
	 * @return double the current market price
	 */
	inline double get_market_price() const;

	/**
	 * @brief Get an iterator to the first bid price level
	 *
	 * @return std::map<double, order_limit>::iterator bid begin
	 * iterator
	 */
	inline std::map<double, order_limit>::iterator bid_limit_begin();

	/**
	 * @brief Get an iterator to the first ask price level
	 *
	 * @return std::map<double, order_limit>::iterator ask begin
	 * iterator
	 */
	inline std::map<double, order_limit>::iterator ask_limit_begin();

	/**
	 * @brief Get an iterator to the end of the bids
	 *
	 * @return std::map<double, order_limit>::iterator bid price
	 * level end iterator
	 */
	inline std::map<double, order_limit>::iterator bid_limit_end();

	/**
	 * @brief Get an iterator to the end of the asks
	 *
	 * @return std::map<double, order_limit>::iterator ask price
	 * level end iterator
	 */
	inline std::map<double, order_limit>::iterator ask_limit_end();

	/**
	 * @brief Get bid price limit at specified price
	 *
	 * @param t_price the price of the bid limit
	 * @return std::map<double, order_limit>::iterator the bid price
	 * limit. Equals bid_limit_end() if this price limit does not
	 * exist.
	 */
	inline std::map<double, order_limit>::iterator bid_limit_at(
	    const double t_price);

	/**
	 * @brief Get ask price limit at specified price
	 *
	 * @param t_price the price of the ask limit
	 * @return std::map<double, order_limit>::iterator the ask price
	 * limit. Equals ask_limit_end() if this price limit does not
	 * exist.
	 */
	inline std::map<double, order_limit>::iterator ask_limit_at(
	    const double t_price);

	~book();

	friend std::ostream &operator<<(std::ostream &t_os, const book &t_book);
	friend order;
	friend trigger;
};

} // namespace elob

#include "common.hpp"
#include "order.hpp"
#include "order_limit.hpp"
#include "trigger.hpp"
#include "trigger_limit.hpp"
#include <iomanip>

std::ostream &elob::operator<<(std::ostream &t_os, const elob::book &t_book) {
	std::size_t w = 12;
	auto bid_it = t_book.m_bids.begin();
	auto ask_it = t_book.m_asks.begin();
	t_os << "┌─────────────────BIDS─────────────────"
	     << "┬─────────────────ASKS─────────────────┐\n";
	t_os << "│ " << std::setw(w) << "PRC" << std::setw(w) << "QTY"
	     << std::setw(w) << "AON QTY"
	     << " │ ";
	t_os << std::setw(w) << "PRC" << std::setw(w) << "QTY" << std::setw(w)
	     << "AON QTY"
	     << " │\n";

	while (true) {

		if (bid_it == t_book.m_bids.end() &&
		    ask_it == t_book.m_asks.end()) {
			break;
		}

		if (bid_it != t_book.m_bids.end()) {
			t_os << "│ " << std::setw(w) << bid_it->first
			     << std::setw(w) << bid_it->second.get_quantity()
			     << std::setw(w)
			     << bid_it->second.get_aon_quantity() << " │ ";
			bid_it++;
		} else {
			t_os << "│ " << std::setw(36) << ' ' << " │ ";
		}

		if (ask_it != t_book.m_asks.end()) {
			if (ask_it->second.order_count() > 0) {
				t_os << std::setw(w) << ask_it->first
				     << std::setw(w)
				     << ask_it->second.get_quantity()
				     << std::setw(w)
				     << ask_it->second.get_aon_quantity()
				     << " │\n";
			}
			ask_it++;
		} else {
			t_os << std::setw(36) << ' ' << " │\n";
		}
	}

	return t_os;
}

void elob::book::insert(elob::c_order_ptr t_order) {
	// check if order is valid
	if (m_order_deferral_depth > 0) {
		m_deferred.push(t_order);
		return;
	}

	begin_order_deferral();

	if (t_order->m_quantity <= 0.0) {
		t_order->on_rejected();
		return;
	}

	if (t_order->m_queued) {
		t_order->on_rejected();
		return;
	}

	// order is valid
	t_order->m_book = this;
	t_order->on_accepted();

	if (t_order->m_side == elob::side::bid) {
		if (t_order->m_all_or_nothing) {
			insert_aon_bid(t_order);
		} else {
			insert_bid(t_order);
		}
	} else {
		if (t_order->m_all_or_nothing) {
			insert_aon_ask(t_order);
		} else {
			insert_ask(t_order);
		}
	}

	end_order_deferral();
}

void elob::book::begin_order_deferral() { ++m_order_deferral_depth; }

void elob::book::end_order_deferral() {
	if (--m_order_deferral_depth != 0) {
		return;
	}

	while (!m_deferred.empty()) {
		auto order_obj = m_deferred.front();
		m_deferred.pop();
		insert(order_obj);
	}
}

void elob::book::insert(elob::c_trigger_ptr t_trigger) {
	// check if order is valid
	if (t_trigger->m_queued) {
		return;
	}

	// order is valid
	t_trigger->m_book = this;
	t_trigger->on_accepted();

	if (t_trigger->m_side == elob::side::bid) {
		if (t_trigger->m_price >= m_market_price &&
		    m_market_price >= 0.0) { // prevent execution at start
			t_trigger->on_triggered();
			t_trigger->m_book = nullptr;
		} else {
			queue_bid_trigger(t_trigger);
		}
	} else {
		if (t_trigger->m_price <= m_market_price) {
			t_trigger->on_triggered();
			t_trigger->m_book = nullptr;
		} else {
			queue_ask_trigger(t_trigger);
		}
	}
}

void elob::book::queue_bid_trigger(elob::c_trigger_ptr &t_trigger) {
	const auto limit_it =
	    m_bid_triggers.emplace(t_trigger->m_price, elob::trigger_limit())
		.first;
	const auto trigger_it = limit_it->second.insert(t_trigger);
	t_trigger->m_limit_it = limit_it;
	t_trigger->m_trigger_it = trigger_it;
	t_trigger->m_queued = true;
	t_trigger->on_queued();
}

void elob::book::queue_ask_trigger(elob::c_trigger_ptr &t_trigger) {
	const auto limit_it =
	    m_ask_triggers.emplace(t_trigger->m_price, elob::trigger_limit())
		.first;
	const auto trigger_it = limit_it->second.insert(t_trigger);
	t_trigger->m_limit_it = limit_it;
	t_trigger->m_trigger_it = trigger_it;
	t_trigger->m_queued = true;
	t_trigger->on_queued();
}

void elob::book::queue_bid_order(elob::c_order_ptr &t_order) {
	const auto limit_it =
	    m_bids.emplace(t_order->m_price, elob::order_limit()).first;
	const auto order_it = limit_it->second.insert(t_order);
	t_order->m_limit_it = limit_it;
	t_order->m_order_it = order_it;
	t_order->m_queued = true;
	check_ask_aons(
	    t_order
		->m_price); // check if any aons on other side can execute now
	t_order->on_queued();
}

void elob::book::queue_ask_order(elob::c_order_ptr &t_order) {
	const auto limit_it =
	    m_asks.emplace(t_order->m_price, elob::order_limit()).first;
	const auto order_it = limit_it->second.insert(t_order);
	t_order->m_limit_it = limit_it;
	t_order->m_order_it = order_it;
	t_order->m_queued = true;
	check_bid_aons(
	    t_order
		->m_price); // check if any aons on other side can execute now
	t_order->on_queued();
}

void elob::book::insert_bid(elob::c_order_ptr &t_order) {

	execute_bid(t_order);

	if (t_order->m_immediate_or_cancel) {
		if (t_order->m_quantity > 0.0) {
			t_order->on_canceled();
		}

		t_order->m_book = nullptr;
		return;
	}

	if (t_order->m_quantity > 0.0) {
		queue_bid_order(t_order);
	} else {
		t_order->m_book = nullptr;
	}
}

void elob::book::insert_ask(elob::c_order_ptr &t_order) {

	execute_ask(t_order);

	if (t_order->m_immediate_or_cancel) {
		if (t_order->m_quantity > 0.0) {
			t_order->on_canceled();
		}

		t_order->m_book = nullptr;
		return;
	}

	if (t_order->m_quantity > 0.0) {
		queue_ask_order(t_order);
	} else {
		t_order->m_book = nullptr;
	}
}

void elob::book::insert_aon_bid(elob::c_order_ptr &t_order) {

	if (bid_is_fillable(t_order)) {
		execute_bid(t_order);
		t_order->m_book = nullptr;
		return;
	}

	if (t_order->m_immediate_or_cancel) {
		t_order->on_canceled();
		t_order->m_book = nullptr;
		return;
	}

	// queue unexecuted aon order
	queue_bid_order(t_order);
}

void elob::book::insert_aon_ask(elob::c_order_ptr &t_order) {

	if (ask_is_fillable(t_order)) {
		execute_ask(t_order);
		t_order->m_book = nullptr;
		return;
	}

	if (t_order->m_immediate_or_cancel) {
		t_order->on_canceled();
		t_order->m_book = nullptr;
		return;
	}

	// queue unexecuted aon order
	queue_ask_order(t_order);
}

bool elob::book::bid_is_fillable(elob::c_order_ptr &t_order) const {
	auto limit_it = m_asks.begin();
	double quantity_remaining = t_order->m_quantity;
	const double order_price = t_order->m_price;

	while (limit_it != m_asks.end() && limit_it->first <= order_price &&
	       quantity_remaining > 0.0) {
		const double limit_quantity = limit_it->second.m_quantity;
		const double aon_limit_quantity =
		    limit_it->second.m_aon_quantity;
		const double total_limit_quantity =
		    limit_quantity + aon_limit_quantity;

		if (quantity_remaining >= total_limit_quantity) {
			quantity_remaining -= total_limit_quantity;
		} else if (quantity_remaining <= limit_quantity) {
			return true;
		} else {
			// computationally expensive
			quantity_remaining =
			    limit_it->second.simulate_trade(quantity_remaining);
		}

		++limit_it;
	}

	return quantity_remaining <= 0.0;
}

bool elob::book::ask_is_fillable(elob::c_order_ptr &t_order) const {
	auto limit_it = m_bids.begin();
	double quantity_remaining = t_order->m_quantity;
	const double order_price = t_order->m_price;

	while (limit_it != m_bids.end() && limit_it->first >= order_price &&
	       quantity_remaining > 0.0) {
		const double limit_quantity = limit_it->second.m_quantity;
		const double aon_limit_quantity =
		    limit_it->second.m_aon_quantity;
		const double total_limit_quantity =
		    limit_quantity + aon_limit_quantity;

		if (quantity_remaining >= total_limit_quantity) {
			quantity_remaining -= total_limit_quantity;
		} else if (quantity_remaining <= limit_quantity) {
			return true;
		} else {
			// computationally expensive
			quantity_remaining =
			    limit_it->second.simulate_trade(quantity_remaining);
		}

		++limit_it;
	}

	return quantity_remaining <= 0.0;
}

void elob::book::execute_bid(elob::c_order_ptr &t_order) {
	auto limit_it = m_asks.begin();
	double order_price = t_order->m_price;

	while (limit_it != m_asks.end() && limit_it->first <= order_price &&
	       t_order->m_quantity > 0.0) {
		if (limit_it->second.trade(t_order) > 0.0) {
			m_market_price = limit_it->first;
		}

		if (limit_it->second.is_empty()) {
			m_asks.erase(limit_it++);
		} else {
			++limit_it;
		}
	}

	auto trigger_limit_it = m_ask_triggers.begin();

	while (trigger_limit_it != m_ask_triggers.end() &&
	       trigger_limit_it->first <= m_market_price) {
		trigger_limit_it->second.trigger_all();
		++trigger_limit_it;
	}

	m_ask_triggers.erase(m_ask_triggers.begin(), trigger_limit_it);
}

void elob::book::execute_ask(elob::c_order_ptr &t_order) {
	auto limit_it = m_bids.begin();
	double order_price = t_order->m_price;

	while (limit_it != m_bids.end() && limit_it->first >= order_price &&
	       t_order->m_quantity > 0.0) {
		if (limit_it->second.trade(t_order) > 0.0) {
			m_market_price = limit_it->first;
		}

		if (limit_it->second.is_empty()) {
			m_bids.erase(limit_it++);
		} else {
			++limit_it;
		}
	}

	auto trigger_limit_it = m_bid_triggers.begin();

	while (trigger_limit_it != m_bid_triggers.end() &&
	       trigger_limit_it->first >= m_market_price) {
		trigger_limit_it->second.trigger_all();
		++trigger_limit_it;
	}

	m_bid_triggers.erase(m_bid_triggers.begin(), trigger_limit_it);
}

void elob::book::execute_queued_aon_bid(elob::c_order_ptr &t_order) {
	const double quantity = t_order->m_quantity;
	execute_bid(t_order);

	t_order->m_limit_it->second.m_aon_quantity -= quantity;
}

void elob::book::execute_queued_aon_ask(elob::c_order_ptr &t_order) {
	const double quantity = t_order->m_quantity;
	execute_ask(t_order);

	t_order->m_limit_it->second.m_aon_quantity -= quantity;
}

void elob::book::check_bid_aons(const double t_price) {
	auto limit_it = m_bids.lower_bound(t_price);

	while (limit_it != m_bids.end()) { // todo check if limit is removed
		auto &limit_obj = limit_it->second;
		auto order_it = limit_obj.m_aon_order_its.begin();
		while (order_it != limit_obj.m_aon_order_its.end()) {
			auto order_obj = **order_it;
			if (bid_is_fillable(order_obj)) {
				execute_queued_aon_bid(order_obj);
				limit_obj.erase(*(order_it++));
			} else {
				++order_it;
			}
		}

		if (limit_it->second.is_empty()) {
			m_bids.erase(limit_it++);
		} else {
			++limit_it;
		}
	}
}

void elob::book::check_ask_aons(const double t_price) {
	auto limit_it = m_asks.lower_bound(t_price);

	while (limit_it != m_asks.end()) { // todo check if limit is removed
		auto &limit_obj = limit_it->second;
		auto order_it = limit_obj.m_aon_order_its.begin();
		while (order_it != limit_obj.m_aon_order_its.end()) {
			auto order_obj = **order_it;
			if (ask_is_fillable(order_obj)) {
				execute_queued_aon_ask(order_obj);
				limit_obj.erase(*(order_it++));
			} else {
				++order_it;
			}
		}

		if (limit_it->second.is_empty()) {
			m_asks.erase(limit_it++);
		} else {
			++limit_it;
		}
	}
}

double elob::book::get_bid_price() const {
	const auto it = m_bids.begin();
	return it != m_bids.end() ? it->first : min_price;
}

double elob::book::get_ask_price() const {
	const auto it = m_asks.begin();
	return it != m_asks.end() ? it->first : max_price;
}

double elob::book::get_market_price() const { return m_market_price; }

std::map<double, elob::order_limit>::iterator elob::book::bid_limit_begin() {
	return m_bids.begin();
}

std::map<double, elob::order_limit>::iterator elob::book::ask_limit_begin() {
	return m_asks.begin();
}

std::map<double, elob::order_limit>::iterator elob::book::bid_limit_end() {
	return m_bids.end();
}

std::map<double, elob::order_limit>::iterator elob::book::ask_limit_end() {
	return m_asks.end();
}

std::map<double, elob::order_limit>::iterator elob::book::bid_limit_at(
    const double t_price) {
	return m_bids.find(t_price);
}

std::map<double, elob::order_limit>::iterator elob::book::ask_limit_at(
    const double t_price) {
	return m_asks.find(t_price);
}

elob::book::~book() {
	m_bids.clear();
	m_asks.clear();

	m_bid_triggers.clear();
	m_ask_triggers.clear();
}

#endif // BOOK_HPP
