module;

#include <functional>
#include <type_traits>
#include <vector>

export module pludux.screener.logical_filter;

import pludux.asset_snapshot;
import pludux.screener.screener_filter;

namespace pludux::screener {

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

  auto operator==(const BinaryLogicalFilter& other) const noexcept
   -> bool = default;

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

  auto operator==(const UnaryLogicalFilter& other) const noexcept
   -> bool = default;

  auto condition() const -> const ScreenerFilter&
  {
    return condition_;
  }

private:
  ScreenerFilter condition_;
};

template<typename T = void>
struct LogicalXor {
  auto operator()(this const auto, T a, T b) -> bool
  {
    return a ? !b : b;
  }
};

template<>
struct LogicalXor<void> {
  auto operator()(this const auto, auto a, auto b)
  {
    return a ? !b : b;
  }
};

export struct AndFilter : BinaryLogicalFilter<AndFilter, std::logical_and<>> {
  using BinaryLogicalFilter<AndFilter, std::logical_and<>>::BinaryLogicalFilter;
};

export struct OrFilter : BinaryLogicalFilter<OrFilter, std::logical_or<>> {
  using BinaryLogicalFilter<OrFilter, std::logical_or<>>::BinaryLogicalFilter;
};

export struct NotFilter : UnaryLogicalFilter<NotFilter, std::logical_not<>> {
  using UnaryLogicalFilter<NotFilter, std::logical_not<>>::UnaryLogicalFilter;
};

export struct XorFilter : BinaryLogicalFilter<XorFilter, LogicalXor<>> {
  using BinaryLogicalFilter<XorFilter, LogicalXor<>>::BinaryLogicalFilter;
};

} // namespace pludux::screener

