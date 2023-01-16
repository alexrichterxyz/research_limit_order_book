#ifndef LIMIT_HPP
#define LIMIT_HPP
#include <list>
#include <memory>

namespace lob {
	class order;
	class trigger;
	class book;

	/**
	 * @brief limits hold the queued order and trigger objects at each price
	 * level.
	 *
	 */
	class limit {
		private:
		double m_quantity = 0.0;
		double m_aon_quantity = 0.0;
		/* orders and triggers are stored in a doubly-linked list to
		 allow for O(1) cancellation.*/
		std::list<std::shared_ptr<order>> m_orders;
		std::list<std::shared_ptr<trigger>> m_triggers;

		/* The field m_aon_order_its stores
		 * iterators to the all-or-nothing orders in m_orders so that
		 * they can be quickly looked up. This is neccessary because
		 * updating order quantities may render some all-or-nothing
		 * orders executable. When all-or-nothing orders are executed or
		 * canceled, their iterators must be deleted from this list
		 * which runs at O(n). This may be fixed in future versions by
		 * storing the iterator to m_aon_order_its in the order objects.
		 * In the meantime it is aniticipated that only few orders will
		 * be all-or-nothing.
		 */
		std::list<std::list<std::shared_ptr<order>>::iterator>
		    m_aon_order_its;

		std::list<std::shared_ptr<order>>::iterator insert(
		    const std::shared_ptr<order> &t_order);
		std::list<std::shared_ptr<trigger>>::iterator insert(
		    const std::shared_ptr<trigger> &t_trigger);

		/**
		 * @brief Simulates the execution of an order with t_quantity
		 * and returns the amount of quantity remaining. This function
		 * is used to test if all-or-nothing orders are fillable.
		 *
		 * @param t_quantity the amount of quantity to be traded.
		 * @return the amount of quantity remaining.
		 */
		double simulate_trade(const double t_quantity) const;

		/**
		 * @brief Execute an inbound order.
		 *
		 * @param t_order, the inbound order
		 * @return the traded quantity.
		 */
		double trade(const std::shared_ptr<order> &t_order);
		inline bool is_empty() const {
			return m_orders.empty() && m_triggers.empty();
		}
		void erase(const std::list<std::shared_ptr<order>>::iterator
			&t_order_it);
		void erase(const std::list<std::shared_ptr<trigger>>::iterator
			&t_trigger_it);
		void trigger_all();

		public:
		/**
		 * @brief Get the non-all-or-none quantity at this price level.
		 * This quantity can be filled partially.
		 *
		 * @return the non-all-or-none quantity at this price level.
		 */
		inline double get_quantity() const { return m_quantity; }

		/**
		 * @brief Get the all-or-none quantity at this price level.
		 *
		 * @return the all-or-none quantity at this price level
		 */
		inline double get_aon_quantity() const {
			return m_aon_quantity;
		}

		/**
		 * @brief Get the total number of orders (all-or-nothing
		 * included) at this price level.
		 *
		 * @return std::size_t, the number of orders (all-or-nothing
		 * included) at this price level.
		 */
		inline std::size_t get_order_count() const {
			return m_orders.size();
		}

		/**
		 * @brief Get an iterator to the first order in the queue.
		 *
		 * @return std::list<std::shared_ptr<order>>::iterator, iterator
		 * to first order in the queue.
		 */
		inline std::list<std::shared_ptr<order>>::iterator
		orders_begin() {
			return m_orders.begin();
		}

		/**
		 * @brief Get an iterator to the end of the order queue.
		 *
		 * @return std::list<std::shared_ptr<order>>::iterator, iterator
		 * to the end of the order queue.
		 */
		inline std::list<std::shared_ptr<order>>::iterator
		orders_end() {
			return m_orders.end();
		}
		
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
		 * @brief Get the number of orders (including all-or-nothing) at this price level.
		 * 
		 * @return the number of orders (including all-or-nothing) at this price level.
		 */
		inline std::size_t order_count() const {
			return m_orders.size();
		}

		/**
		 * @brief Get the number of all-or-nothing-orders at this price level
		 * 
		 * @return the number of all-or-nothing orders at this price level.
		 */
		inline std::size_t aon_order_count() const {
			return m_aon_order_its.size();
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
		friend order;
		friend trigger;

		~limit();
	};
} // namespace lob

#endif // #ifndef LIMIT_HPP