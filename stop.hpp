#ifndef STOP_HPP
#define STOP_HPP
#include "order.hpp"
#include "trigger.hpp"
#include <memory>

namespace elob {

	template <class order_t> class stop : virtual public trigger {
		private:
		std::shared_ptr<order_t> m_order;

		void on_triggered() override;

		public:
		stop(side t_side, double t_price,
		    std::shared_ptr<order_t> t_order);

		inline const std::shared_ptr<order_t> &
		get_pending_order() const {
			return m_order;
		};
	};

	typedef stop<order> stop_order;
	typedef stop<trigger> stop_trigger;
} // namespace elob

#endif // #ifndef STOP_HPP

#include "stop.tpp"