#ifndef TRAILING_STOP_HPP
#define TRAILING_STOP_HPP
#include "order.hpp"
#include "trigger.hpp"
#include <memory>

namespace lob {

	template <class order_t> class trailing_stop : virtual public trigger {
		private:
		offset_type m_offset_type;
		double m_offset;
		std::shared_ptr<order_t> m_order;
		std::shared_ptr<trigger> m_trailing_stop_controller;
		bool initialized = false;

		protected:
		void on_triggered() override;
		void on_queued() override;
		void on_canceled() override;

		public:
		trailing_stop(const side t_side, const double t_price,
		    const offset_type t_offset_type, const double t_offset,
		    std::shared_ptr<order> t_order);

		inline const std::shared_ptr<order> &get_pending_order() const {
			return m_order;
		};
	};

	typedef trailing_stop<order> trailing_stop_order;
	typedef trailing_stop<trigger> trailing_stop_trigger;

	class trailing_stop_controller : virtual public trigger {
		private:
		offset_type m_offset_type;
		double m_offset;
		trigger &m_trailing_stop;
		void on_triggered() override;

		public:
		trailing_stop_controller(const side t_side,
		    const double t_price, const offset_type t_offset_type,
		    const double t_offset, trigger &t_trailing_stop_order);
	};
} // namespace lob

#endif // #ifndef TRAILING_STOP_HPP

#include "trailing_stop.tpp"