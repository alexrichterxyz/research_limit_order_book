#ifndef STOP_HPP
#define STOP_HPP
#include "order.hpp"
#include "trigger.hpp"
#include <memory>

namespace elob {

template <class order_t> class stop : virtual public trigger {
	private:
	std::shared_ptr<order_t> m_order;

	void on_triggered() override;

	public:
	stop(side t_side, double t_price, std::shared_ptr<order_t> t_order);

	inline const std::shared_ptr<order_t> &get_pending_order() const;
};

typedef stop<order> stop_order;
typedef stop<trigger> stop_trigger;
} // namespace elob

#include "book.hpp"

template <class order_t>
elob::stop<order_t>::stop(
    elob::side t_side, double t_price, std::shared_ptr<order_t> t_order)
    : elob::trigger(t_side, t_price), m_order(t_order) {}

template <class order_t> void elob::stop<order_t>::on_triggered() {
	get_book()->insert(m_order);
}

template <class order_t>
const std::shared_ptr<order_t> &elob::stop<order_t>::get_pending_order() const {
	return m_order;
};

#endif // #ifndef STOP_HPP