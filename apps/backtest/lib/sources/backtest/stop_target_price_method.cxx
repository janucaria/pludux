module;

#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <string_view>
#include <utility>

export module pludux.backtest:stop_target_price_method;

import pludux;

import :position_method;

export namespace pludux::backtest {

template<typename TAmountMethod = ValueMethod>
class SlAmountMethod {
public:
  SlAmountMethod()
  : SlAmountMethod{0.0}
  {
  }

  explicit SlAmountMethod(double amount)
  : SlAmountMethod{ValueMethod{amount}}
  {
  }

  explicit SlAmountMethod(TAmountMethod amount)
  : amount_{std::move(amount)}
  {
  }

  auto operator==(const SlAmountMethod&) const noexcept -> bool = default;

  auto amount(this const SlAmountMethod& self) noexcept -> const TAmountMethod&
  {
    return self.amount_;
  }

private:
  TAmountMethod amount_;
};

template<typename TAmountMethod = ValueMethod>
class TpAmountMethod {
public:
  TpAmountMethod()
  : TpAmountMethod{0.0}
  {
  }

  explicit TpAmountMethod(double amount)
  : TpAmountMethod{ValueMethod{amount}}
  {
  }

  explicit TpAmountMethod(TAmountMethod amount)
  : amount_{std::move(amount)}
  {
  }

  auto operator==(const TpAmountMethod&) const noexcept -> bool = default;

  auto amount(this const TpAmountMethod& self) noexcept -> const TAmountMethod&
  {
    return self.amount_;
  }

private:
  TAmountMethod amount_;
};

template<typename TPercentMethod = ValueMethod>
class SlPercentMethod {
public:
  SlPercentMethod()
  : SlPercentMethod{0.0}
  {
  }

  explicit SlPercentMethod(double percent)
  : SlPercentMethod{ValueMethod{percent}}
  {
  }

  explicit SlPercentMethod(TPercentMethod percent)
  : percent_{std::move(percent)}
  {
  }

  auto operator==(const SlPercentMethod&) const noexcept -> bool = default;

  auto percent(this const SlPercentMethod& self) noexcept
   -> const TPercentMethod&
  {
    return self.percent_;
  }

private:
  TPercentMethod percent_;
};

template<typename TPercentMethod = ValueMethod>
class TpPercentMethod {
public:
  TpPercentMethod()
  : TpPercentMethod{0.0}
  {
  }

  explicit TpPercentMethod(double percent)
  : TpPercentMethod{ValueMethod{percent}}
  {
  }

  explicit TpPercentMethod(TPercentMethod percent)
  : percent_{std::move(percent)}
  {
  }

  auto operator==(const TpPercentMethod&) const noexcept -> bool = default;

  auto percent(this const TpPercentMethod& self) noexcept
   -> const TPercentMethod&
  {
    return self.percent_;
  }

private:
  TPercentMethod percent_;
};

template<typename TPeriodMethod = ValueMethod,
         typename TMultiplierMethod = ValueMethod>
class SlAtrMethod {
public:
  SlAtrMethod()
  : SlAtrMethod{14.0, 2.0}
  {
  }

  SlAtrMethod(double period,
              double multiplier,
              MaMethodType ma_smoothing_type = MaMethodType::Rma)
  : SlAtrMethod{ValueMethod{period}, ValueMethod{multiplier}, ma_smoothing_type}
  {
  }

  SlAtrMethod(TPeriodMethod period,
              TMultiplierMethod multiplier,
              MaMethodType ma_smoothing_type = MaMethodType::Rma)
  : period_{std::move(period)}
  , multiplier_{std::move(multiplier)}
  , ma_smoothing_type_{ma_smoothing_type}
  {
  }

  auto operator==(const SlAtrMethod&) const noexcept -> bool = default;

  auto period(this const SlAtrMethod& self) noexcept -> const TPeriodMethod&
  {
    return self.period_;
  }

  auto multiplier(this const SlAtrMethod& self) noexcept
   -> const TMultiplierMethod&
  {
    return self.multiplier_;
  }

  auto ma_smoothing_type(this const SlAtrMethod& self) noexcept -> MaMethodType
  {
    return self.ma_smoothing_type_;
  }

private:
  TPeriodMethod period_;
  TMultiplierMethod multiplier_;
  MaMethodType ma_smoothing_type_;
};

template<typename TPeriodMethod = ValueMethod,
         typename TMultiplierMethod = ValueMethod>
class TpAtrMethod {
public:
  TpAtrMethod()
  : TpAtrMethod{14.0, 2.0}
  {
  }

  TpAtrMethod(double period,
              double multiplier,
              MaMethodType ma_smoothing_type = MaMethodType::Rma)
  : TpAtrMethod{ValueMethod{period}, ValueMethod{multiplier}, ma_smoothing_type}
  {
  }

  TpAtrMethod(TPeriodMethod period,
              TMultiplierMethod multiplier,
              MaMethodType ma_smoothing_type = MaMethodType::Rma)
  : period_{std::move(period)}
  , multiplier_{std::move(multiplier)}
  , ma_smoothing_type_{ma_smoothing_type}
  {
  }

  auto operator==(const TpAtrMethod&) const noexcept -> bool = default;

  auto period(this const TpAtrMethod& self) noexcept -> const TPeriodMethod&
  {
    return self.period_;
  }

  auto multiplier(this const TpAtrMethod& self) noexcept
   -> const TMultiplierMethod&
  {
    return self.multiplier_;
  }

  auto ma_smoothing_type(this const TpAtrMethod& self) noexcept -> MaMethodType
  {
    return self.ma_smoothing_type_;
  }

private:
  TPeriodMethod period_;
  TMultiplierMethod multiplier_;
  MaMethodType ma_smoothing_type_;
};

class Sl1RMethod {
public:
  auto operator==(const Sl1RMethod&) const noexcept -> bool = default;
};

template<typename TMultipleMethod = ValueMethod>
class TpRMultipleMethod {
public:
  TpRMultipleMethod()
  : TpRMultipleMethod{2.0}
  {
  }

  explicit TpRMultipleMethod(double multiple)
  : TpRMultipleMethod{ValueMethod{multiple}}
  {
  }

  explicit TpRMultipleMethod(TMultipleMethod multiple)
  : multiple_{std::move(multiple)}
  {
  }

  auto operator==(const TpRMultipleMethod&) const noexcept -> bool = default;

  auto multiple(this const TpRMultipleMethod& self) noexcept
   -> const TMultipleMethod&
  {
    return self.multiple_;
  }

private:
  TMultipleMethod multiple_;
};

template<typename TMethod>
auto hash_series_method(const SlAmountMethod<TMethod>& method) noexcept
 -> std::size_t
{
  return std::hash<std::string_view>{}("pludux.backtest.SlAmountMethod") ^
         hash_series_method(method.amount());
}

template<typename TMethod>
auto hash_series_method(const TpAmountMethod<TMethod>& method) noexcept
 -> std::size_t
{
  return std::hash<std::string_view>{}("pludux.backtest.TpAmountMethod") ^
         hash_series_method(method.amount());
}

template<typename TMethod>
auto hash_series_method(const SlPercentMethod<TMethod>& method) noexcept
 -> std::size_t
{
  return std::hash<std::string_view>{}("pludux.backtest.SlPercentMethod") ^
         hash_series_method(method.percent());
}

template<typename TMethod>
auto hash_series_method(const TpPercentMethod<TMethod>& method) noexcept
 -> std::size_t
{
  return std::hash<std::string_view>{}("pludux.backtest.TpPercentMethod") ^
         hash_series_method(method.percent());
}

template<typename TPeriodMethod, typename TMultiplierMethod>
auto hash_series_method(
 const SlAtrMethod<TPeriodMethod, TMultiplierMethod>& method) noexcept
 -> std::size_t
{
  return std::hash<std::string_view>{}("pludux.backtest.SlAtrMethod") ^
         hash_series_method(method.period()) ^
         hash_series_method(method.multiplier()) ^
         std::hash<int>{}(static_cast<int>(method.ma_smoothing_type()));
}

template<typename TPeriodMethod, typename TMultiplierMethod>
auto hash_series_method(
 const TpAtrMethod<TPeriodMethod, TMultiplierMethod>& method) noexcept
 -> std::size_t
{
  return std::hash<std::string_view>{}("pludux.backtest.TpAtrMethod") ^
         hash_series_method(method.period()) ^
         hash_series_method(method.multiplier()) ^
         std::hash<int>{}(static_cast<int>(method.ma_smoothing_type()));
}

auto hash_series_method(const Sl1RMethod&) noexcept -> std::size_t
{
  return std::hash<std::string_view>{}("pludux.backtest.Sl1RMethod");
}

template<typename TMethod>
auto hash_series_method(const TpRMultipleMethod<TMethod>& method) noexcept
 -> std::size_t
{
  return std::hash<std::string_view>{}("pludux.backtest.TpRMultipleMethod") ^
         hash_series_method(method.multiple());
}

auto stop_target_price(double reference_price,
                       double distance,
                       double sign,
                       double direction) -> double
{
  if(!std::isfinite(reference_price) || !std::isfinite(direction) ||
     direction == 0.0 || !std::isfinite(distance)) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  return reference_price + sign * direction * distance;
}

template<typename TMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const SlAmountMethod<TMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto amount =
   evaluate_series_method(method.amount(), asset_snapshot, context);
  return stop_target_price(backtest_position_reference_price(context),
                           amount,
                           -1.0,
                           backtest_position_direction(context));
}

template<typename TMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const TpAmountMethod<TMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto amount =
   evaluate_series_method(method.amount(), asset_snapshot, context);
  return stop_target_price(backtest_position_reference_price(context),
                           amount,
                           1.0,
                           backtest_position_direction(context));
}

template<typename TMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const SlPercentMethod<TMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto reference_price = backtest_position_reference_price(context);
  const auto percent =
   evaluate_series_method(method.percent(), asset_snapshot, context);
  return stop_target_price(reference_price,
                           reference_price * percent / 100.0,
                           -1.0,
                           backtest_position_direction(context));
}

template<typename TMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const TpPercentMethod<TMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto reference_price = backtest_position_reference_price(context);
  const auto percent =
   evaluate_series_method(method.percent(), asset_snapshot, context);
  return stop_target_price(reference_price,
                           reference_price * percent / 100.0,
                           1.0,
                           backtest_position_direction(context));
}

template<typename TPeriodMethod, typename TMultiplierMethod>
auto pludux_tag_invoke(
 EvaluateSeriesMethod,
 const SlAtrMethod<TPeriodMethod, TMultiplierMethod>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  const auto atr = evaluate_series_method(
   AtrMethod{method.period(), method.ma_smoothing_type()},
   asset_snapshot,
   context);
  const auto multiplier =
   evaluate_series_method(method.multiplier(), asset_snapshot, context);
  return stop_target_price(backtest_position_reference_price(context),
                           atr * multiplier,
                           -1.0,
                           backtest_position_direction(context));
}

template<typename TPeriodMethod, typename TMultiplierMethod>
auto pludux_tag_invoke(
 EvaluateSeriesMethod,
 const TpAtrMethod<TPeriodMethod, TMultiplierMethod>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  const auto atr = evaluate_series_method(
   AtrMethod{method.period(), method.ma_smoothing_type()},
   asset_snapshot,
   context);
  const auto multiplier =
   evaluate_series_method(method.multiplier(), asset_snapshot, context);
  return stop_target_price(backtest_position_reference_price(context),
                           atr * multiplier,
                           1.0,
                           backtest_position_direction(context));
}

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const Sl1RMethod&,
                       AssetSnapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto risk_distance = backtest_position_risk_distance(context);
  if(!std::isfinite(risk_distance) || risk_distance <= 0.0) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  return stop_target_price(backtest_position_reference_price(context),
                           risk_distance,
                           -1.0,
                           backtest_position_direction(context));
}

template<typename TMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const TpRMultipleMethod<TMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto risk_distance = backtest_position_risk_distance(context);
  const auto multiple =
   evaluate_series_method(method.multiple(), asset_snapshot, context);
  if(!std::isfinite(risk_distance) || risk_distance <= 0.0 ||
     !std::isfinite(multiple) || multiple <= 0.0) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  return stop_target_price(backtest_position_reference_price(context),
                           risk_distance * multiple,
                           1.0,
                           backtest_position_direction(context));
}

} // namespace pludux::backtest
