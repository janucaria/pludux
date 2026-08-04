module;

#include <cstddef>
#include <functional>
#include <limits>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

export module pludux.backtest:risk_distance_method;

import pludux;

import :backtest_method_context;

export namespace pludux::backtest {

template<typename TAmountMethod = ValueMethod>
class RiskDistanceAmountMethod {
public:
  RiskDistanceAmountMethod()
  : RiskDistanceAmountMethod{1.0}
  {
  }

  explicit RiskDistanceAmountMethod(double amount)
  : RiskDistanceAmountMethod{ValueMethod{amount}}
  {
  }

  explicit RiskDistanceAmountMethod(TAmountMethod amount)
  : amount_{std::move(amount)}
  {
  }

  auto operator==(const RiskDistanceAmountMethod&) const noexcept
   -> bool = default;

  auto amount(this const RiskDistanceAmountMethod& self) noexcept
   -> const TAmountMethod&
  {
    return self.amount_;
  }

private:
  TAmountMethod amount_;
};

template<typename TPercentMethod = ValueMethod>
class RiskDistancePercentMethod {
public:
  RiskDistancePercentMethod()
  : RiskDistancePercentMethod{1.0}
  {
  }

  explicit RiskDistancePercentMethod(double percent)
  : RiskDistancePercentMethod{ValueMethod{percent}}
  {
  }

  explicit RiskDistancePercentMethod(TPercentMethod percent)
  : percent_{std::move(percent)}
  {
  }

  auto operator==(const RiskDistancePercentMethod&) const noexcept
   -> bool = default;

  auto percent(this const RiskDistancePercentMethod& self) noexcept
   -> const TPercentMethod&
  {
    return self.percent_;
  }

private:
  TPercentMethod percent_;
};

template<typename TPeriodMethod = ValueMethod,
         typename TMultiplierMethod = ValueMethod>
class RiskDistanceAtrMethod {
public:
  RiskDistanceAtrMethod()
  : RiskDistanceAtrMethod{14.0, 2.0}
  {
  }

  RiskDistanceAtrMethod(double period,
                        double multiplier,
                        MaMethodType ma_smoothing_type = MaMethodType::Rma)
  : RiskDistanceAtrMethod{
     ValueMethod{period}, ValueMethod{multiplier}, ma_smoothing_type}
  {
  }

  RiskDistanceAtrMethod(TPeriodMethod period,
                        TMultiplierMethod multiplier,
                        MaMethodType ma_smoothing_type = MaMethodType::Rma)
  : period_{std::move(period)}
  , multiplier_{std::move(multiplier)}
  , ma_smoothing_type_{ma_smoothing_type}
  {
  }

  auto operator==(const RiskDistanceAtrMethod&) const noexcept
   -> bool = default;

  auto period(this const RiskDistanceAtrMethod& self) noexcept
   -> const TPeriodMethod&
  {
    return self.period_;
  }

  auto multiplier(this const RiskDistanceAtrMethod& self) noexcept
   -> const TMultiplierMethod&
  {
    return self.multiplier_;
  }

  auto ma_smoothing_type(this const RiskDistanceAtrMethod& self) noexcept
   -> MaMethodType
  {
    return self.ma_smoothing_type_;
  }

private:
  TPeriodMethod period_;
  TMultiplierMethod multiplier_;
  MaMethodType ma_smoothing_type_;
};

template<typename TMethod>
auto hash_series_method(
 const RiskDistanceAmountMethod<TMethod>& method) noexcept -> std::size_t
{
  return std::hash<std::string_view>{}(
          "pludux.backtest.RiskDistanceAmountMethod") ^
         hash_series_method(method.amount());
}

template<typename TMethod>
auto hash_series_method(
 const RiskDistancePercentMethod<TMethod>& method) noexcept -> std::size_t
{
  return std::hash<std::string_view>{}(
          "pludux.backtest.RiskDistancePercentMethod") ^
         hash_series_method(method.percent());
}

template<typename TPeriodMethod, typename TMultiplierMethod>
auto hash_series_method(
 const RiskDistanceAtrMethod<TPeriodMethod, TMultiplierMethod>& method) noexcept
 -> std::size_t
{
  return std::hash<std::string_view>{}(
          "pludux.backtest.RiskDistanceAtrMethod") ^
         hash_series_method(method.period()) ^
         hash_series_method(method.multiplier()) ^
         std::hash<int>{}(static_cast<int>(method.ma_smoothing_type()));
}

auto risk_distance_reference_price(MethodContextable auto context) noexcept
 -> double
{
  if constexpr(std::is_same_v<std::monostate, decltype(context)>) {
    return std::numeric_limits<double>::quiet_NaN();
  } else if constexpr(std::is_same_v<std::remove_cvref_t<decltype(context)>,
                                     BacktestMethodContext>) {
    return context.position_reference_price();
  } else if constexpr(std::is_same_v<std::remove_cvref_t<decltype(context)>,
                                     ErasedSeriesMethodContext>) {
    const auto* backtest_context =
     series_method_context_cast<BacktestMethodContext>(context);
    return backtest_context ? backtest_context->position_reference_price()
                            : std::numeric_limits<double>::quiet_NaN();
  } else {
    return std::numeric_limits<double>::quiet_NaN();
  }
}

template<typename TMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const RiskDistanceAmountMethod<TMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  return evaluate_series_method(method.amount(), asset_snapshot, context);
}

template<typename TMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const RiskDistancePercentMethod<TMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto reference_price = risk_distance_reference_price(context);
  const auto percent =
   evaluate_series_method(method.percent(), asset_snapshot, context);
  return reference_price * percent / 100.0;
}

template<typename TPeriodMethod, typename TMultiplierMethod>
auto pludux_tag_invoke(
 EvaluateSeriesMethod,
 const RiskDistanceAtrMethod<TPeriodMethod, TMultiplierMethod>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  const auto atr = evaluate_series_method(
   AtrMethod{method.period(), method.ma_smoothing_type()},
   asset_snapshot,
   context);
  const auto multiplier =
   evaluate_series_method(method.multiplier(), asset_snapshot, context);
  return atr * multiplier;
}

} // namespace pludux::backtest
