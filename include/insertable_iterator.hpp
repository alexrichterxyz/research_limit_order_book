#ifndef INSERTABLE_ITERATOR_HPP
#define INSERTABLE_ITERATOR_HPP
#include <list>
#include <map>

namespace elob {
class book;
template <class Cmp, class Lim, class Ins> class insertable_iterator {
	private:
	std::map<double, Lim, Cmp> &m_side;
	typename std::map<double, Lim, Cmp>::iterator m_limit_it;
	typename std::list<Ins>::iterator m_insertable_it;

	insertable_iterator(std::map<double, Lim, Cmp> &t_side,
	    const typename std::map<double, Lim, Cmp>::iterator &t_limit_it,
	    const typename std::list<Ins>::iterator &t_insertable_it);

	insertable_iterator(std::map<double, Lim, Cmp> &t_side,
	    const typename std::map<double, Lim, Cmp>::iterator &t_limit_it);

	public:
	insertable_iterator(const insertable_iterator &t_other);

	bool operator==(const insertable_iterator &t_other);

	bool operator!=(const insertable_iterator &t_other);

	insertable_iterator<Cmp, Lim, Ins> operator++();
	insertable_iterator<Cmp, Lim, Ins> operator++(int);

	typename std::list<Ins>::iterator operator->();
	Ins &operator*();

	friend book;
};

} // namespace elob

#include <functional>
#include <iostream>

template <class Cmp, class Lim, class Ins>
elob::insertable_iterator<Cmp, Lim, Ins>::insertable_iterator(
    std::map<double, Lim, Cmp> &t_side,
    const typename std::map<double, Lim, Cmp>::iterator &t_limit_it,
    const typename std::list<Ins>::iterator &t_insertable_it)
    : m_side(t_side), m_limit_it(t_limit_it), m_insertable_it(t_insertable_it) {}

template <class Cmp, class Lim, class Ins>
elob::insertable_iterator<Cmp, Lim, Ins>::insertable_iterator(
    std::map<double, Lim, Cmp> &t_side,
    const typename std::map<double, Lim, Cmp>::iterator &t_limit_it)
    : m_side(t_side), m_limit_it(t_limit_it) {}

template <class Cmp, class Lim, class Ins>
elob::insertable_iterator<Cmp, Lim, Ins>::insertable_iterator(
    const insertable_iterator<Cmp, Lim, Ins> &t_other)
    : m_side(t_other.m_side), m_limit_it(t_other.m_limit_it),
      m_insertable_it(t_other.m_insertable_it) {}

template <class Cmp, class Lim, class Ins>
bool elob::insertable_iterator<Cmp, Lim, Ins>::operator==(
    const insertable_iterator<Cmp, Lim, Ins> &t_other) {

	// do not check insertable iterator if limit iterator is end
	if (m_side.end() == m_limit_it) {
		return true;
	}

	// if not end, checking the insertable iterator is sufficient
	return m_insertable_it == t_other.m_insertable_it;
}

template <class Cmp, class Lim, class Ins>
bool elob::insertable_iterator<Cmp, Lim, Ins>::operator!=(
    const insertable_iterator<Cmp, Lim, Ins> &t_other) {

	return !(*this == t_other);
}

template <class Cmp, class Lim, class Ins>
elob::insertable_iterator<Cmp, Lim, Ins>
elob::insertable_iterator<Cmp, Lim, Ins>::operator++() {
	if (m_limit_it != m_side.end()) {
		if (m_insertable_it != std::prev(m_limit_it->second.end())) {
			++m_insertable_it;
		} else if (++m_limit_it != m_side.end()) {
			m_insertable_it = m_limit_it->second.begin();
		}
	}

	return *this;
}

template <class Cmp, class Lim, class Ins>
elob::insertable_iterator<Cmp, Lim, Ins>
elob::insertable_iterator<Cmp, Lim, Ins>::operator++(int) {
	auto pre_increment_copy = *this;

	if (m_limit_it != m_side.end()) {
		if (m_insertable_it != std::prev(m_limit_it->second.end())) {
			++m_insertable_it;
		} else if (++m_limit_it != m_side.end()) {
			m_insertable_it = m_limit_it->second.begin();
		}
	}

	return pre_increment_copy;
}

template <class Cmp, class Lim, class Ins>
typename std::list<Ins>::iterator
elob::insertable_iterator<Cmp, Lim, Ins>::operator->() {
	return m_insertable_it;
}

template <class Cmp, class Lim, class Ins>
Ins &elob::insertable_iterator<Cmp, Lim, Ins>::operator*() {
	return *m_insertable_it;
}

#endif // #ifndef INSERTABLE_ITERATOR_HPP