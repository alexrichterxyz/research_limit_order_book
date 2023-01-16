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
    const double t_offset, std::shared_ptr<elob::order> t_order)
    : elob::trigger(t_side, t_price), m_offset_type(t_offset_type),
      m_offset(t_offset), m_order(t_order) {}

template <class order_t> void elob::trailing_stop<order_t>::on_triggered() {
	get_book()->insert(m_order);
	m_trailing_stop_controller->cancel();
	std::cout << "Triggered trailing stop order\n";
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