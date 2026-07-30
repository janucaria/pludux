module;

#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

export module pludux:methods.bb_method;

import :methods.ohlcv_method;
import :methods.adaptive_ma_method;
import :methods.value_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod,
         typename TParameterMethod = ValueMethod>
class BbMethod {
public:
  BbMethod()
  : BbMethod{20, 1.5}
  {
  }

  BbMethod(std::size_t period, double stddev)
  : BbMethod{TSourceMethod{}, period, stddev}
  {
  }

  BbMethod(TSourceMethod source,
           std::size_t period,
           double stddev,
           MaMethodType ma_method_type = MaMethodType::Sma)
  : BbMethod{std::move(source),
             ValueMethod{static_cast<double>(period)},
             ValueMethod{stddev},
             ma_method_type}
  {
  }

  BbMethod(TSourceMethod source,
           TParameterMethod period,
           TParameterMethod stddev,
           MaMethodType ma_method_type = MaMethodType::Sma)
    requires(!std::is_arithmetic_v<TParameterMethod>)
  : source_{std::move(source)}
  , period_{std::move(period)}
  , stddev_{std::move(stddev)}
  , ma_method_type_{ma_method_type}
  {
  }

  auto operator==(const BbMethod& other) const noexcept -> bool = default;

  auto source(this const BbMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this BbMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto ma_method_type(this const BbMethod& self) noexcept -> MaMethodType
  {
    return self.ma_method_type_;
  }

  auto ma_method_type(this BbMethod& self, MaMethodType ma_method_type) noexcept
  {
    self.ma_method_type_ = ma_method_type;
  }

  auto period(this const BbMethod& self) noexcept -> const TParameterMethod&
  {
    return self.period_;
  }

  void period(this BbMethod& self, TParameterMethod new_period) noexcept
  {
    self.period_ = std::move(new_period);
  }

  auto stddev(this const BbMethod& self) noexcept -> const TParameterMethod&
  {
    return self.stddev_;
  }

  void stddev(this BbMethod& self, TParameterMethod new_stddev) noexcept
  {
    self.stddev_ = std::move(new_stddev);
  }

private:
  TSourceMethod source_;
  TParameterMethod period_;
  TParameterMethod stddev_;
  MaMethodType ma_method_type_;
};

} // namespace pludux
