#ifndef ORDER_HPP
#define ORDER_HPP
#include "common.hpp"
#include "order_limit.hpp"
#include <list>
#include <map>
#include <memory>

namespace lob {

	class book;

	/**
	 * @brief the order class defines the fundamental properties of orders
	 * including side, price, quantity, and whether the order is immediate
	 * or cancel, or all or nothing. Market orders are represented as limit
	 * orders with a price of zero (sell) or infinity (buy). Objects of this
	 * class can be inserted into book objects. The behavior of orders can
	 * be customized by overriding the virtual event methods: on_queued,
	 * on_accepted, on_rejected, on_traded, and on_canceled.
	 *
	 */
	class order : public std::enable_shared_from_this<order> {
		private:
		const side m_side;
		const double m_price;
		double m_quantity = 0.0;
		const bool m_immediate_or_cancel = false;
		bool m_all_or_nothing = false;
		bool m_queued = false;

		/* pointer to the book into which the order was inserted.
		 it's guaranteed to be dereferencable in the virtual event
		 methods. */
		book *m_book = nullptr;

		/* these iterators store the location of the order in the order
		 book. They are used to cancel the order in O(1). */
		std::map<double, lob::order_limit>::iterator m_limit_it;
		std::list<std::shared_ptr<order>>::iterator m_order_it;

		protected:
		/**
		 * @brief book. At this stage the order has been verified to be
		 * valid and is awaiting execution. This event method can be
		 * used to adapt the price of the order to the current market
		 * price of the book.
		 *
		 */
		virtual void on_accepted(){};

		/**
		 * @brief called once the order has been queued at the specified
		 * price level.
		 *
		 */
		virtual void on_queued(){};

		/**
		 * @brief called if the order was rejected by the book. This may
		 * happen if the order is already queued, has a negative price,
		 * etc.
		 *
		 */
		virtual void on_rejected(){};

		/**
		 * @brief called after the order executed against another one.
		 *
		 * @param t_order the other order
		 */
		virtual void on_traded(const std::shared_ptr<order> &t_order){};

		/**
		 * @brief called once the order got canceled. This may happen if
		 * the order got canceled manually or if the order is immediate
		 * or cancel and could not get filled immediately.
		 *
		 */
		virtual void on_canceled(){};

		public:
		/**
		 * @brief Construct a new order object
		 *
		 * @param t_side the side at which the order will be inserted
		 * (either lob::side::bid or lob::side::ask)
		 * @param t_price the price at which the order will be inserted.
		 * For market orders this will be 0.0 (sell) or DBL_MAX (buy)
		 * @param t_quantity the quantity demanded or provided.
		 * @param t_immediate_or_cancel indicator of whether the order
		 * is immediate or cancel.
		 * @param t_all_or_nothing indicator of whether the order is all
		 * or nothing.
		 */
		order(const side t_side, const double t_price,
		    const double t_quantity, const bool t_immediate_or_cancel=false,
		    const bool t_all_or_nothing=false);

		/**
		 * @brief Cancels the order, if possible. Currently, only queued
		 * orders can be canceled.
		 *
		 * @return true successfully cancelled.
		 * @return false could not cancel the order because it hasn't
		 * been queued yet.
		 */
		bool cancel();

		/**
		 * @brief Get the instance of the book into which the order was
		 * inserted. This value is guaranteed to be non-nullptr in the
		 * virtual event methods.
		 *
		 * @return book* pointer to the book object into which the order
		 * was inserted or nullptr if it hasn't been inserted into a
		 * book yet or got removed from it.
		 */
		inline book *get_book() const { return m_book; }

		/**
		 * @brief Get the side of the order.
		 *
		 * @return either lob::side::bid or lob::side::ask.
		 */
		inline side get_side() const { return m_side; }

		/**
		 * @brief Get the price of the order.
		 *
		 * @return double price of the order.
		 */
		inline double get_price() const { return m_price; }

		/**
		 * @brief Get the quantity of the order.
		 *
		 * @return the quantity of the order.
		 */
		inline double get_quantity() const { return m_quantity; }

		/**
		 * @brief Update the quantity of the order. This operation is
		 * O(1) in some cases but can be very inefficient if there are
		 * lot of all or nothing orders in the book.
		 *
		 * @param t_quantity
		 */
		void set_quantity(const double t_quantity);

		/**
		 * @brief Check if the order is immediate or cancel.
		 *
		 * @return true, is immediate or cancel.
		 * @return false, is not immediate or cancel.
		 */
		inline bool is_immediate_or_cancel() const {
			return m_immediate_or_cancel;
		}

		/**
		 * @brief Check if the order is all or nothing.
		 *
		 * @return true, is all or nothing.
		 * @return false, is not all or nothing.
		 */
		inline bool is_all_or_nothing() const {
			return m_all_or_nothing;
		}

		/**
		 * @brief Update the order's all or nothing flag.
		 * 
		 * @param t_all_or_nothing the update value of the order's all or nothing flag
		 */
		void set_all_or_nothing(const bool t_all_or_nothing);

		/**
		 * @brief Check whether the order is queued. Queued orders can be canceled.
		 * 
		 * @return true, the order is queued.
		 * @return false, the order is not queued.
		 */
		inline bool is_queued() const { return m_queued; }

		friend book;
		friend order_limit;
	};

} // namespace lob

#endif // #ifndef ORDER_HPP