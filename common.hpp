#ifndef COMMON_HPP
#define COMMON_HPP
#include <cfloat>
#include <memory>
namespace elob {

enum side { bid = 0, ask };

enum offset_type { abs = 0, pct };

const double max_price = DBL_MAX;
const double min_price = 0.0;

class order;
using order_ptr = std::shared_ptr<order>;
using c_order_ptr = const order_ptr;

class trigger;
using trigger_ptr = std::shared_ptr<trigger>;
using c_trigger_ptr = const trigger_ptr;

} // namespace elob

#endif // #ifndef COMMON_HPP