#ifndef COMMON_HPP
#define COMMON_HPP
#include <cfloat>

namespace elob {

	enum side { bid = 0, ask };

	enum offset_type { abs = 0, pct };

	const double max_price = DBL_MAX;
	const double min_price = 0.0;
} // namespace elob

#endif // #ifndef COMMON_HPP