module;

#include <functional>
#include <utility>

export module pludux:methods.logical_method;

export namespace pludux {

template<typename TOperator,
         typename TFirstCondition,
         typename TSecondCondition>
class BinaryLogicalMethod {
public:
  BinaryLogicalMethod(TFirstCondition first_condition,
                      TSecondCondition second_condition)
  : first_condition_{std::move(first_condition)}
  , second_condition_{std::move(second_condition)}
  {
  }

  auto operator==(const BinaryLogicalMethod& other) const noexcept
   -> bool = default;

  auto first_condition(this const BinaryLogicalMethod& self)
   -> const TFirstCondition&
  {
    return self.first_condition_;
  }

  void first_condition(this BinaryLogicalMethod& self,
                       TFirstCondition condition)
  {
    self.first_condition_ = std::move(condition);
  }

  auto second_condition(this const BinaryLogicalMethod& self)
   -> const TSecondCondition&
  {
    return self.second_condition_;
  }

  void second_condition(this BinaryLogicalMethod& self,
                        TSecondCondition condition)
  {
    self.second_condition_ = std::move(condition);
  }

private:
  TFirstCondition first_condition_;
  TSecondCondition second_condition_;
};

template<typename TOperator, typename TOtherCondition>
class UnaryLogicalMethod {
public:
  explicit UnaryLogicalMethod(TOtherCondition condition)
  : other_condition_{std::move(condition)}
  {
  }

  auto operator==(const UnaryLogicalMethod& other) const noexcept
   -> bool = default;

  auto other_condition(this const UnaryLogicalMethod& self)
   -> const TOtherCondition&
  {
    return self.other_condition_;
  }

  void other_condition(this UnaryLogicalMethod& self, TOtherCondition condition)
  {
    self.other_condition_ = std::move(condition);
  }

private:
  TOtherCondition other_condition_;
};

template<typename T = void>
struct LogicalXor {
  auto operator()(this const LogicalXor, T a, T b) -> bool
  {
    return a ? !b : b;
  }
};

template<>
struct LogicalXor<void> {
  auto operator()(this const LogicalXor, auto a, auto b)
  {
    return a ? !b : b;
  }
};

template<typename TFirstCondition, typename TSecondCondition>
using LogicalAndMethod =
 BinaryLogicalMethod<std::logical_and<>, TFirstCondition, TSecondCondition>;

template<typename TFirstCondition, typename TSecondCondition>
using LogicalOrMethod =
 BinaryLogicalMethod<std::logical_or<>, TFirstCondition, TSecondCondition>;

template<typename TOtherCondition>
using LogicalNotMethod =
 UnaryLogicalMethod<std::logical_not<>, TOtherCondition>;

template<typename TFirstCondition, typename TSecondCondition>
using LogicalXorMethod =
 BinaryLogicalMethod<LogicalXor<>, TFirstCondition, TSecondCondition>;

} // namespace pludux
