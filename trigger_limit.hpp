#ifndef TRIGGER_LIMIT_HPP
#define TRIGGER_LIMIT_HPP
#include <list>
#include <memory>

namespace elob {
	class trigger;
	class book;

	class trigger_limit {
		private:
		std::list<std::shared_ptr<trigger>> m_triggers;

		std::list<std::shared_ptr<trigger>>::iterator insert(
		    const std::shared_ptr<trigger> &t_trigger);

		inline bool is_empty() const { return m_triggers.empty(); }

		void erase(const std::list<std::shared_ptr<trigger>>::iterator
			&t_trigger_it);
		void trigger_all();

		public:
		/**
		 * @brief Get an iterator to the first trigger in the queue.
		 *
		 * @return std::list<std::shared_ptr<trigger>>::iterator,
		 * iterator to first trigger in the queue.
		 */
		inline std::list<std::shared_ptr<trigger>>::iterator
		triggers_begin();

		/**
		 * @brief Get an iterator to the end of the trigger queue.
		 *
		 * @return std::list<std::shared_ptr<trigger>>::iterator,
		 * iterator to the end of the trigger queue.
		 */
		inline std::list<std::shared_ptr<trigger>>::iterator
		triggers_end();

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

std::list<std::shared_ptr<elob::trigger>>::iterator elob::trigger_limit::insert(
    const std::shared_ptr<elob::trigger> &t_trigger) {
	m_triggers.push_back(t_trigger);
	return std::prev(m_triggers.end());
}

void elob::trigger_limit::erase(
    const std::list<std::shared_ptr<elob::trigger>>::iterator &t_trigger_it) {
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
	for (auto &trigger : m_triggers) {
		trigger->m_book = nullptr;
		trigger->m_queued = false;
	}
}

std::list<std::shared_ptr<elob::trigger>>::iterator
elob::trigger_limit::triggers_begin() {
	return m_triggers.begin();
}

inline std::list<std::shared_ptr<elob::trigger>>::iterator
elob::trigger_limit::triggers_end() {
	return m_triggers.end();
}

inline std::size_t elob::trigger_limit::trigger_count() const {
	return m_triggers.size();
}

#endif // #ifndef TRIGGER_LIMIT_HPP