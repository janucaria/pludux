module;

#include <functional>
#include <type_traits>
#include <vector>

export module pludux:conditions.logical_method;

import :asset_snapshot;
import :series.method_contextable;

import :conditions.any_condition_method;

namespace pludux {

template<typename, typename TBinaryLogicalOperator>
  requires std::is_invocable_r_v<bool, TBinaryLogicalOperator, bool, bool>
class BinaryLogicalMethod {
public:
  BinaryLogicalMethod(AnyConditionMethod first_condition,
                      AnyConditionMethod second_condition)
  : first_condition_{std::move(first_condition)}
  , second_condition_{std::move(second_condition)}
  {
  }

  auto operator==(const BinaryLogicalMethod& other) const noexcept
   -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) -> bool
  {
    const auto first_condition = self.first_condition_(asset_snapshot, context);
    const auto second_condition =
     self.second_condition_(asset_snapshot, context);

    return TBinaryLogicalOperator{}(first_condition, second_condition);
  }

  auto first_condition(this const auto& self) -> const AnyConditionMethod&
  {
    return self.first_condition_;
  }

  auto second_condition(this const auto& self) -> const AnyConditionMethod&
  {
    return self.second_condition_;
  }

private:
  AnyConditionMethod first_condition_;
  AnyConditionMethod second_condition_;
};

template<typename T, typename TUnaryLogicalOperator>
  requires std::is_invocable_r_v<bool, TUnaryLogicalOperator, bool>
class UnaryLogicalMethod {
public:
  explicit UnaryLogicalMethod(AnyConditionMethod condition)
  : other_condition_{std::move(condition)}
  {
  }

  auto operator==(const UnaryLogicalMethod& other) const noexcept
   -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) -> bool
  {
    const auto condition = self.other_condition_(asset_snapshot, context);
    return TUnaryLogicalOperator{}(condition);
  }

  auto other_condition(this const auto& self) -> const AnyConditionMethod&
  {
    return self.other_condition_;
  }

private:
  AnyConditionMethod other_condition_;
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

export struct AndMethod : BinaryLogicalMethod<AndMethod, std::logical_and<>> {
  AndMethod(AnyConditionMethod first, AnyConditionMethod second)
  : BinaryLogicalMethod<AndMethod, std::logical_and<>>(std::move(first),
                                                       std::move(second))
  {
  }
};

export struct OrMethod : BinaryLogicalMethod<OrMethod, std::logical_or<>> {
  OrMethod(AnyConditionMethod first, AnyConditionMethod second)
  : BinaryLogicalMethod<OrMethod, std::logical_or<>>(std::move(first),
                                                     std::move(second))
  {
  }
};

export struct NotMethod : UnaryLogicalMethod<NotMethod, std::logical_not<>> {
  NotMethod(AnyConditionMethod condition)
  : UnaryLogicalMethod<NotMethod, std::logical_not<>>(std::move(condition))
  {
  }
};

export struct XorMethod : BinaryLogicalMethod<XorMethod, LogicalXor<>> {
  XorMethod(AnyConditionMethod first, AnyConditionMethod second)
  : BinaryLogicalMethod<XorMethod, LogicalXor<>>(std::move(first),
                                                 std::move(second))
  {
  }
};

} // namespace pludux
