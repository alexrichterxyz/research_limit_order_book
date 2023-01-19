#ifndef TRIGGER_HPP
#define TRIGGER_HPP
#include "common.hpp"
#include <list>
#include <map>
#include <memory>

namespace elob {

	class trigger_limit;
	class book;

	/**
	 * @brief An object of class trigger is essentially an event handler
	 * that gets triggered once the market price rises above or falls below
	 * the specified price. Triggers inserted on the bid side respond to
	 * falling prices whereas triggers inserted on the ask side respond to
	 * rising prices. Triggers are essential building blocks for
	 * market-price-based order types such as stop orders. Just like order
	 * objects, triggers can be inserted into the order book. Two major
	 * differences between triggers and order objects are: triggers can move
	 * themselves to different price levels, triggers get inserted into the
	 * order book immediately whereas orders may get deferred.
	 *
	 */
	class trigger : public std::enable_shared_from_this<trigger> {
		private:
		const side m_side;
		double m_price;
		bool m_queued = false;

		/* pointer to the book into which the order was inserted.
			it's guaranteed to be dereferencable in the virtual
		   event methods. */
		book *m_book = nullptr;

		/* these iterators store the location of the order in the order
			book. They are used to cancel the order in O(1). */
		std::map<double, elob::trigger_limit>::iterator m_limit_it;
		std::list<std::shared_ptr<trigger>>::iterator m_trigger_it;

		protected:
		/**
		 * @brief book. At this stage the trigger has been verified to
		 * be valid and is about to be inserted. This event method can
		 * be used to adapt the price of the trigger to the current
		 * market price of the book.
		 *
		 */
		virtual void on_accepted(){};

		/**
		 * @brief called once the trigglimit
		 */
		virtual void on_queued(){};

		/**
		 * @brief called if the trigger was rejected by the book. This
		 * may happen if the trigger is already queued, has a negative
		 * price, etc.
		 *
		 */
		virtual void on_rejected(){};

		/**
		 * @brief triggers on the ask side get triggered if the market
		 * price (price of the last trade) rises above or reaches the
		 * specified price. Triggers inserted on the bid side are
		 * responsive to falling prices.
		 *
		 */
		virtual void on_triggered(){};

		/**
		 * @brief called once the trigger got canceled.
		 *
		 */
		virtual void on_canceled(){};

		public:
		/**
		 * @brief Get the price of the order.
		 *
		 * @return double price of the order.
		 */
		inline double get_price() const;

		/**
		 * @brief Update the price of the trigger.
		 *
		 * @param t_price, the new price of the trigger.
		 */
		inline void set_price(double t_price);

		/**
		 * @brief Get the side of the trigger.
		 *
		 * @return either elob::side::bid or elob::side:ask
		 */
		inline side get_side() const;

		/**
		 * @brief Construct a new trigger object
		 *
		 * @param t_side, either elob::side::bid or elob::side::ask. Bid
		 * triggers are responsive to falling market prices whereas ask
		 * triggers are responsive to rising market prices.
		 * @param t_price, the market price (price of last trade) at
		 * which the on_triggered method will be triggered.
		 */
		trigger(side t_side, double t_price);

		/**
		 * @brief Get the instance of the book into which the TRIGGER
		 * was inserted. This value is guaranteed to be non-nullptr in
		 * the virtual event methods.
		 *
		 * @return book* pointer to the book object into which the
		 * TRIGGER was inserted or nullptr if it hasn't been inserted
		 * into a book yet or got removed from it.
		 */
		inline book *get_book() const;

		/**
		 * @brief Cancels the TRIGGER, if possible. Currently, only
		 * queued triggers can be canceled.
		 *
		 * @return true successfully cancelled.
		 * @return false could not cancel the trigger because it hasn't
		 * been queued yet.
		 */
		inline bool cancel();

		/**
		 * @brief Check whether the tigger is queued. Queued triggers
		 * can be canceled.
		 *
		 * @return true, the trigger is queued.
		 * @return false, the trigger is not queued.
		 */
		inline bool is_queued() const;

		friend book;
		friend trigger_limit;
	};
} // namespace elob

#include "book.hpp"
#include "trigger_limit.hpp"

elob::trigger::trigger(elob::side t_side, double t_price)
    : m_side(t_side), m_price(t_price) {}

bool elob::trigger::cancel() {
	if (m_queued) {

		m_limit_it->second.erase(m_trigger_it);

		if (m_limit_it->second.is_empty()) {
			if (m_side == side::bid) {
				m_book->m_bid_triggers.erase(m_limit_it);
			} else {
				m_book->m_ask_triggers.erase(m_limit_it);
			}
		}

		on_canceled();

		if (!m_queued) { // on_canceled may reinsert the trigger
			m_book = nullptr;
		}

		return true;
	}

	return false;
}

void elob::trigger::set_price(double t_price) {
	if (m_price == t_price) {
		return;
	}

	if (m_queued) {
		m_limit_it->second.erase(m_trigger_it);

		if (m_limit_it->second.is_empty()) {
			if (m_side == side::bid) {
				m_book->m_bid_triggers.erase(m_limit_it);
			} else {
				m_book->m_ask_triggers.erase(m_limit_it);
			}
		}
	}

	m_price = t_price;
	m_book->insert(shared_from_this());
}

double elob::trigger::get_price() const { return m_price; }

elob::side elob::trigger::get_side() const { return m_side; }

elob::book *elob::trigger::get_book() const { return m_book; }

bool elob::trigger::is_queued() const { return m_queued; }

#endif // #ifndef TRIGGER_HPP