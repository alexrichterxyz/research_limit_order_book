#ifndef TRIGGER_LIMIT_HPP
#define TRIGGER_LIMIT_HPP
#include <list>
#include <memory>

namespace elob {
class trigger;
class book;

class trigger_limit {
	private:
	std::list<trigger_ptr> m_triggers;

	std::list<trigger_ptr>::iterator insert(c_trigger_ptr &t_trigger);

	inline bool is_empty() const { return m_triggers.empty(); }

	void erase(const std::list<trigger_ptr>::iterator &t_trigger_it);
	void trigger_all();

	public:
	/**
	 * @brief Get an iterator to the first trigger in the queue.
	 *
	 * @return std::list<elob::trigger_ptr>::iterator,
	 * iterator to first trigger in the queue.
	 */
	inline std::list<trigger_ptr>::iterator triggers_begin();

	/**
	 * @brief Get an iterator to the end of the trigger queue.
	 *
	 * @return std::list<elob::trigger_ptr>::iterator,
	 * iterator to the end of the trigger queue.
	 */
	inline std::list<trigger_ptr>::iterator triggers_end();

	/**
	 * @brief Get the number of triggers at this price level.
	 *
	 * @return the number of tiggers at this price level.
	 */
	inline std::size_t trigger_count() const;

	friend book;
	friend trigger;

	~trigger_limit();
};

} // namespace elob

std::list<elob::trigger_ptr>::iterator elob::trigger_limit::insert(
    elob::c_trigger_ptr &t_trigger) {
	m_triggers.push_back(t_trigger);
	return std::prev(m_triggers.end());
}

void elob::trigger_limit::erase(
    const std::list<elob::trigger_ptr>::iterator &t_trigger_it) {
	auto trigger_obj = *t_trigger_it;
	trigger_obj->m_queued = false;
	m_triggers.erase(t_trigger_it);
}

void elob::trigger_limit::trigger_all() {

	while (!m_triggers.empty()) {
		auto trigger_obj = m_triggers.front();
		m_triggers.pop_front();
		trigger_obj->m_queued = false;
		trigger_obj->on_triggered();

		if (!trigger_obj
			 ->m_queued) { // on_triggered may reinsert the trigger
			trigger_obj->m_book = nullptr;
		}
	}
}

elob::trigger_limit::~trigger_limit() {
	for (const auto &trigger : m_triggers) {
		trigger->m_book = nullptr;
		trigger->m_queued = false;
	}
}

std::list<elob::trigger_ptr>::iterator elob::trigger_limit::triggers_begin() {
	return m_triggers.begin();
}

inline std::list<elob::trigger_ptr>::iterator
elob::trigger_limit::triggers_end() {
	return m_triggers.end();
}

inline std::size_t elob::trigger_limit::trigger_count() const {
	return m_triggers.size();
}

#endif // #ifndef TRIGGER_LIMIT_HPP