#ifndef TRAILING_STOP_HPP
#define TRAILING_STOP_HPP
#include "order.hpp"
#include "trigger.hpp"
#include <memory>

namespace elob {

template <class order_t> class trailing_stop : virtual public trigger {
	private:
	offset_type m_offset_type;
	double m_offset;
	std::shared_ptr<order_t> m_order;
	trigger_ptr m_trailing_stop_controller;
	bool initialized = false;

	protected:
	void on_triggered() override;
	void on_queued() override;
	void on_canceled() override;

	public:
	trailing_stop(const side t_side, const double t_price,
	    const offset_type t_offset_type, const double t_offset,
	    elob::order_ptr t_order);

	inline const std::shared_ptr<order_t> &get_pending_order() const;
};

typedef trailing_stop<order> trailing_stop_order;
typedef trailing_stop<trigger> trailing_stop_trigger;

class trailing_stop_controller : virtual public trigger {
	private:
	offset_type m_offset_type;
	double m_offset;
	trigger &m_trailing_stop;
	void on_triggered() override;

	public:
	trailing_stop_controller(const side t_side, const double t_price,
	    const offset_type t_offset_type, const double t_offset,
	    trigger &t_trailing_stop_order);
};
} // namespace elob

#include "book.hpp"
#include <cmath>

elob::trailing_stop_controller::trailing_stop_controller(const side t_side,
    const double t_price, const elob::offset_type t_offset_type,
    const double t_offset, elob::trigger &t_trailing_stop)
    : trigger(t_side, t_price), m_offset_type(t_offset_type),
      m_offset(t_offset), m_trailing_stop(t_trailing_stop) {}

void elob::trailing_stop_controller::on_triggered() {
	const double market_price = get_book()->get_market_price();
	double new_price = 0.0;
	double new_stop_price = 0.0;

	if (get_side() == side::ask) {
		new_price = std::nextafter(market_price, max_price);

		if (m_offset_type == elob::offset_type::abs) {
			new_stop_price = market_price - m_offset;
		} else {
			new_stop_price = market_price * (1 - m_offset);
		}

		new_stop_price =
		    std::max(new_stop_price, m_trailing_stop.get_price());
	} else {
		new_price = std::nextafter(market_price, min_price);

		if (m_offset_type == elob::offset_type::abs) {
			new_stop_price = market_price + m_offset;
		} else {
			new_stop_price = market_price * (1 + m_offset);
		}

		new_stop_price =
		    std::min(new_stop_price, m_trailing_stop.get_price());
	}

	set_price(new_price);
	m_trailing_stop.set_price(new_stop_price);
}

template <class order_t>
elob::trailing_stop<order_t>::trailing_stop(const elob::side t_side,
    const double t_price, const elob::offset_type t_offset_type,
    const double t_offset, elob::order_ptr t_order)
    : elob::trigger(t_side, t_price), m_offset_type(t_offset_type),
      m_offset(t_offset), m_order(t_order) {}

template <class order_t>
const std::shared_ptr<order_t> &
elob::trailing_stop<order_t>::get_pending_order() const {
	return m_order;
};

template <class order_t> void elob::trailing_stop<order_t>::on_triggered() {
	get_book()->insert(m_order);
	m_trailing_stop_controller->cancel();
}

template <class order_t> void elob::trailing_stop<order_t>::on_queued() {
	if (initialized) {
		return;
	}

	// not initialized yet
	initialized = true;

	const double market_price = get_book()->get_market_price();

	// if trailing stop gets triggered on price falls,
	// then it will get updated on price rises and vice versa.
	// Thus, the controller is on the opposite side of the book
	elob::side controller_side = elob::side::bid;
	double controller_price = 0.0;

	if (get_side() == elob::side::bid) {
		controller_side = elob::side::ask;
		controller_price = std::nextafter(market_price, max_price);
	} else {
		controller_side = elob::side::bid;
		controller_price = std::nextafter(market_price, min_price);
	}

	m_trailing_stop_controller = std::make_shared<trailing_stop_controller>(
	    controller_side, controller_price, m_offset_type, m_offset, *this);

	get_book()->insert(m_trailing_stop_controller);
}

template <class order_t> void elob::trailing_stop<order_t>::on_canceled() {
	initialized = false;
	m_trailing_stop_controller->cancel();
}

#endif // #ifndef TRAILING_STOP_HPP