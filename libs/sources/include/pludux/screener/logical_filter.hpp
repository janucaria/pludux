#ifndef PLUDUX_PLUDUX_SCREENER_LOGICAL_FILTER_HPP
#define PLUDUX_PLUDUX_SCREENER_LOGICAL_FILTER_HPP

#include <functional>
#include <type_traits>
#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/screener_filter.hpp>

import pludux.asset_snapshot;

namespace pludux::screener {

namespace detail {

template<typename, typename TBinaryLogicalOperator>
  requires std::is_invocable_r_v<bool, TBinaryLogicalOperator, bool, bool>
class BinaryLogicalFilter {
public:
  BinaryLogicalFilter(ScreenerFilter first_condition,
                      ScreenerFilter second_condition)
  : first_condition_{std::move(first_condition)}
  , second_condition_{std::move(second_condition)}
  {
  }

  auto operator()(AssetSnapshot asset_data) const -> bool
  {
    const auto first_condition = first_condition_(asset_data);
    const auto second_condition = second_condition_(asset_data);

    return TBinaryLogicalOperator{}(first_condition, second_condition);
  }

  auto operator==(const BinaryLogicalFilter& other) const noexcept -> bool = default;

  auto first_condition() const -> const ScreenerFilter&
  {
    return first_condition_;
  }

  auto second_condition() const -> const ScreenerFilter&
  {
    return second_condition_;
  }

private:
  ScreenerFilter first_condition_;
  ScreenerFilter second_condition_;
};

template<typename T, typename TUnaryLogicalOperator>
  requires std::is_invocable_r_v<bool, TUnaryLogicalOperator, bool>
class UnaryLogicalFilter {
public:
  UnaryLogicalFilter(ScreenerFilter condition)
  : condition_{std::move(condition)}
  {
  }

  auto operator()(AssetSnapshot asset_data) const -> bool
  {
    const auto condition = condition_(asset_data);
    return TUnaryLogicalOperator{}(condition);
  }

  auto operator==(const UnaryLogicalFilter& other) const noexcept -> bool = default;

  auto condition() const -> const ScreenerFilter&
  {
    return condition_;
  }

private:
  ScreenerFilter condition_;
};

template<typename T = void>
struct LogicalXor {
  auto operator()(T a, T b) const -> bool
  {
    return a ? !b : b;
  }
};

template<>
struct LogicalXor<void> {
  auto operator()(auto a, auto b) const
  {
    return a ? !b : b;
  }
};

} // namespace detail

struct AndFilter : detail::BinaryLogicalFilter<AndFilter, std::logical_and<>> {
  using detail::BinaryLogicalFilter<AndFilter,
                                    std::logical_and<>>::BinaryLogicalFilter;
};

struct OrFilter : detail::BinaryLogicalFilter<OrFilter, std::logical_or<>> {
  using detail::BinaryLogicalFilter<OrFilter,
                                    std::logical_or<>>::BinaryLogicalFilter;
};

struct NotFilter : detail::UnaryLogicalFilter<NotFilter, std::logical_not<>> {
  using detail::UnaryLogicalFilter<NotFilter,
                                   std::logical_not<>>::UnaryLogicalFilter;
};

struct XorFilter
: detail::BinaryLogicalFilter<XorFilter, detail::LogicalXor<>> {
  using detail::BinaryLogicalFilter<XorFilter,
                                    detail::LogicalXor<>>::BinaryLogicalFilter;
};

} // namespace pludux::screener

#endif
