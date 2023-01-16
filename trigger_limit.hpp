#ifndef TRIGGER_LIMIT_HPP
#define TRIGGER_LIMIT_HPP
#include <list>
#include <memory>

namespace lob {
    class trigger;
    class book;

    class trigger_limit {
        private:
		std::list<std::shared_ptr<trigger>> m_triggers;

		std::list<std::shared_ptr<trigger>>::iterator insert(
		    const std::shared_ptr<trigger> &t_trigger);

		inline bool is_empty() const {
			return m_triggers.empty();
		}

		void erase(const std::list<std::shared_ptr<trigger>>::iterator
			&t_trigger_it);
		void trigger_all();

		public:
		/**
		 * @brief Get an iterator to the first trigger in the queue.
		 *
		 * @return std::list<std::shared_ptr<trigger>>::iterator, iterator
		 * to first trigger in the queue.
		 */
		inline std::list<std::shared_ptr<trigger>>::iterator
		triggers_begin() {
			return m_triggers.begin();
		}

		/**
		 * @brief Get an iterator to the end of the trigger queue.
		 *
		 * @return std::list<std::shared_ptr<trigger>>::iterator, iterator
		 * to the end of the trigger queue.
		 */
		inline std::list<std::shared_ptr<trigger>>::iterator
		triggers_end() {
			return m_triggers.end();
		}

		/**
		 * @brief Get the number of triggers at this price level.
		 * 
		 * @return the number of tiggers at this price level.
		 */
		inline std::size_t trigger_count() const {
			return m_triggers.size();
		}

		friend book;
		friend trigger;

		~trigger_limit();
    };

} // namespace lob

#endif // #ifndef TRIGGER_LIMIT_HPP