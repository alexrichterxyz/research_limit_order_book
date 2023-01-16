#include "book.hpp"
#include "order_limit.hpp"
#include "trigger_limit.hpp"
#include <iomanip>

std::ostream &elob::operator<<(std::ostream &t_os, const elob::book &t_book) {
	std::size_t w = 12;
	auto bid_it = t_book.m_bids.begin();
	auto ask_it = t_book.m_asks.begin();
	t_os << "┌─────────────────BIDS─────────────────┬─────────────────ASKS─"
		"────────────────┐\n";
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

void elob::book::insert(const std::shared_ptr<order> t_order) {
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

void elob::book::insert(const std::shared_ptr<trigger> t_trigger) {
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

void elob::book::queue_bid_trigger(const std::shared_ptr<trigger> &t_trigger) {
	const auto limit_it =
	    m_bid_triggers.emplace(t_trigger->m_price, elob::trigger_limit())
		.first;
	const auto trigger_it = limit_it->second.insert(t_trigger);
	t_trigger->m_limit_it = limit_it;
	t_trigger->m_trigger_it = trigger_it;
	t_trigger->m_queued = true;
	t_trigger->on_queued();
}

void elob::book::queue_ask_trigger(const std::shared_ptr<trigger> &t_trigger) {
	const auto limit_it =
	    m_ask_triggers.emplace(t_trigger->m_price, elob::trigger_limit())
		.first;
	const auto trigger_it = limit_it->second.insert(t_trigger);
	t_trigger->m_limit_it = limit_it;
	t_trigger->m_trigger_it = trigger_it;
	t_trigger->m_queued = true;
	t_trigger->on_queued();
}

void elob::book::queue_bid_order(const std::shared_ptr<order> &t_order) {
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

void elob::book::queue_ask_order(const std::shared_ptr<order> &t_order) {
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

void elob::book::insert_bid(const std::shared_ptr<order> &t_order) {

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

void elob::book::insert_ask(const std::shared_ptr<order> &t_order) {

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

void elob::book::insert_aon_bid(const std::shared_ptr<order> &t_order) {

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

void elob::book::insert_aon_ask(const std::shared_ptr<order> &t_order) {

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

bool elob::book::bid_is_fillable(const std::shared_ptr<order> &t_order) const {
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

bool elob::book::ask_is_fillable(const std::shared_ptr<order> &t_order) const {
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

void elob::book::execute_bid(const std::shared_ptr<order> &t_order) {
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

void elob::book::execute_ask(const std::shared_ptr<order> &t_order) {
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

void elob::book::execute_queued_aon_bid(const std::shared_ptr<order> &t_order) {
	const double quantity = t_order->m_quantity;
	execute_bid(t_order);

	t_order->m_limit_it->second.m_aon_quantity -= quantity;
}

void elob::book::execute_queued_aon_ask(const std::shared_ptr<order> &t_order) {
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

elob::book::~book() {
	m_bids.clear();
	m_asks.clear();

	m_bid_triggers.clear();
	m_ask_triggers.clear();
}