#include "book.hpp"

template <class order_t>
lob::stop<order_t>::stop(
    lob::side t_side, double t_price, std::shared_ptr<lob::order_t> t_order)
    : lob::trigger(t_side, t_price), m_order(t_order) {}

template <class order_t>
void lob::stop<order_t>::on_triggered() { get_book()->insert(m_order); }