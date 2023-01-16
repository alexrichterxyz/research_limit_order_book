#ifndef BOOK_HPP
#define BOOK_HPP
#include "common.hpp"
#include "order.hpp"
#include "order_limit.hpp"
#include "trigger_limit.hpp"
#include "trigger.hpp"
#include <map>
#include <memory>
#include <queue>

namespace lob {
	class book;

	std::ostream &operator<<(std::ostream &t_os, const book &t_book);

	/**
	 * @brief book implements a price-time-priotity matching engine. Orders
	 * and triggers can be inserted into book objects.
	 *
	 */
	class book {
		private:
		/* During order execution. event handlers like "on_trade" are
		 * called which may insert additional orders recursively. These
		 * additional orders will be deferred. Only once
		 * the outer insertion call has completed, the additional orders
		 * are removed from the deferral queue and executed. */
		std::size_t m_order_deferral_depth = 0;
		std::queue<std::shared_ptr<order>> m_deferred;

		std::map<double, order_limit, std::greater<double>> m_bids;
		std::map<double, order_limit, std::less<double>> m_asks;

		std::map<double, trigger_limit, std::greater<double>> m_bid_triggers;
		std::map<double, trigger_limit, std::less<double>> m_ask_triggers;

		// set to -1 to prevent triggers from being triggered
		// immediately.
		double m_market_price = -1.0;

		/**
		 * \internal
		 * @brief When called, subsequent orders will be deferred rather
		 * than being queued immediately. This is required to ensure
		 * orders are fully executed before new orders, e.g. those
		 * inserted from within event handlers, are executed.
		 *
		 */
		void begin_order_deferral();

		/**
		 * \internal
		 * @brief Once the outer insertion call has been completed,
		 * orders from the deferral queue are executed.
		 *
		 */
		void end_order_deferral();

		void insert_bid(const std::shared_ptr<order> &t_order);
		void insert_ask(const std::shared_ptr<order> &t_order);

		void insert_aon_bid(const std::shared_ptr<order> &t_order);
		void insert_aon_ask(const std::shared_ptr<order> &t_order);

		/**
		 * \internal
		 * @brief Check if the bid order can be filled completely. This
		 * check is performed before all-or-nothing orders are executed.
		 *
		 * @param t_order the (all-or-nothing) bid order to be executed.
		 * @return true the order is completely fillable
		 * @return false the order is only partially fillable
		 */
		bool bid_is_fillable(
		    const std::shared_ptr<order> &t_order) const;

		/**
		 * \internal
		 * @brief Check if the ask order can be filled completely. This
		 * check is performed before all-or-nothing orders are executed.
		 *
		 * @param t_order the (all-or-nothing) ask order to be executed.
		 * @return true the order is completely fillable
		 * @return false the order is only partially fillable
		 */
		bool ask_is_fillable(
		    const std::shared_ptr<order> &t_order) const;

		void execute_bid(const std::shared_ptr<order> &t_order);
		void execute_ask(const std::shared_ptr<order> &t_order);

		void execute_queued_aon_bid(
		    const std::shared_ptr<order> &t_order);
		void execute_queued_aon_ask(
		    const std::shared_ptr<order> &t_order);

		void queue_bid_order(const std::shared_ptr<order> &t_order);
		void queue_ask_order(const std::shared_ptr<order> &t_order);

		void queue_bid_trigger(
		    const std::shared_ptr<trigger> &t_trigger);
		void queue_ask_trigger(
		    const std::shared_ptr<trigger> &t_trigger);

		/**
		 * @brief Check if any all-or-nothing bids at the specified
		 * price or lower are executable. This function is called if the
		 * quantity of queued orders is increased.
		 *
		 * @param t_price the price from which queued all-or-nothing
		 * will be checked.
		 */
		void check_bid_aons(
		    const double t_price); // check if executable

		/**
		 * @brief Check if any all-or-nothing asks at the specified
		 * price or higher are executable. This function is called if
		 * the quantity of queued orders is increased.
		 *
		 * @param t_price the price from which queued all-or-nothing
		 * will be checked.
		 */
		void check_ask_aons(
		    const double t_price); // check if executable

		public:
		/**
		 * @brief Inserts an order into the book. Marketable orders will
		 * be executed. Partially filled orders will be queued (or
		 * canelled if marked as immediate-or-cancel). When the function
		 * is called from within another order's event handler (like
		 * on_trade), the order will be deferred and only executed once
		 * the other order has been handled.
		 *
		 * @param t_order the order to be inserted
		 */
		void insert(const std::shared_ptr<order> t_order);

		/**
		 * @brief Inserts a trigger into the book. Unlike orders,
		 * triggers will cannot be deferred and will instead be queued
		 * immediately.
		 *
		 * @param t_trigger the trigger to be inserted
		 */
		void insert(const std::shared_ptr<trigger> t_trigger);

		/**
		 * @brief Get the best bid price.
		 *
		 * @return double the best bid price
		 */
		inline double get_bid_price() const {
			const auto it = m_bids.begin();
			return it != m_bids.end() ? it->first : min_price;
		}

		/**
		 * @brief Get the best ask price.
		 *
		 * @return double the best ask price
		 */
		inline double get_ask_price() const {
			const auto it = m_asks.begin();
			return it != m_asks.end() ? it->first : max_price;
		}

		/**
		 * @brief Get the price at which the last trade occured.
		 *
		 * @return double the current market price
		 */
		inline double get_market_price() const {
			return m_market_price;
		}

		/**
		 * @brief Get an iterator to the first bid price level
		 *
		 * @return std::map<double, order_limit>::iterator bid begin iterator
		 */
		inline std::map<double, order_limit>::iterator bid_limit_begin() {
			return m_bids.begin();
		}

		/**
		 * @brief Get an iterator to the first ask price level
		 *
		 * @return std::map<double, order_limit>::iterator ask begin iterator
		 */
		inline std::map<double, order_limit>::iterator ask_limit_begin() {
			return m_asks.begin();
		}

		/**
		 * @brief Get an iterator to the end of the bids
		 *
		 * @return std::map<double, order_limit>::iterator bid price level end
		 * iterator
		 */
		inline std::map<double, order_limit>::iterator bid_limit_end() {
			return m_bids.end();
		}

		/**
		 * @brief Get an iterator to the end of the asks
		 *
		 * @return std::map<double, order_limit>::iterator ask price level end
		 * iterator
		 */
		inline std::map<double, order_limit>::iterator ask_limit_end() {
			return m_asks.end();
		}

		/**
		 * @brief Get bid price limit at specified price
		 *
		 * @param t_price the price of the bid limit
		 * @return std::map<double, order_limit>::iterator the bid price
		 * limit. Equals bid_limit_end() if this price limit does not
		 * exist.
		 */
		inline std::map<double, order_limit>::iterator bid_limit_at(
		    const double t_price) {
			return m_bids.find(t_price);
		}

		/**
		 * @brief Get ask price limit at specified price
		 *
		 * @param t_price the price of the ask limit
		 * @return std::map<double, order_limit>::iterator the ask price
		 * limit. Equals ask_limit_end() if this price limit does not
		 * exist.
		 */
		inline std::map<double, order_limit>::iterator ask_limit_at(
		    const double t_price) {
			return m_asks.find(t_price);
		}

		~book();

		friend std::ostream &operator<<(
		    std::ostream &t_os, const book &t_book);
		friend order;
		friend trigger;
	};

} // namespace lob

#endif // BOOK_HPP