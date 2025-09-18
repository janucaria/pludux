#ifndef PLUDUX_PLUDUX_SCREENER_ABS_DIFF_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_ABS_DIFF_METHOD_HPP

#include <cstddef>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/arithmetic_method.hpp>
#include <pludux/series.hpp>

import pludux.asset_snapshot;

namespace pludux::screener {

namespace details {
template<typename T = void>
struct AbsoluteDifference {
  auto operator()(T left, T right) const -> T
  {
    return std::abs(left - right);
  }
};

template<>
struct AbsoluteDifference<void> {
  auto operator()(auto left, auto right) const
  {
    return std::abs(left - right);
  }
};
} // namespace details

class AbsDiffMethod
: public details::BinaryArithmeticMethod<AbsDiffMethod,
                                         details::AbsoluteDifference<>> {
public:
  using details::BinaryArithmeticMethod<
   AbsDiffMethod,
   details::AbsoluteDifference<>>::BinaryArithmeticMethod;

  auto minuend() const noexcept -> const ScreenerMethod&;

  auto subtrahend() const noexcept -> const ScreenerMethod&;
};

} // namespace pludux::screener

#endif
