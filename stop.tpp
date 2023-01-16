#include "book.hpp"

template <class order_t>
elob::stop<order_t>::stop(
    elob::side t_side, double t_price, std::shared_ptr<elob::order_t> t_order)
    : elob::trigger(t_side, t_price), m_order(t_order) {}

template <class order_t>
void elob::stop<order_t>::on_triggered() { get_book()->insert(m_order); }