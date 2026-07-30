module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:methods.macd_method;

import :methods.ohlcv_method;
import :methods.value_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod,
         typename TParameterMethod = ValueMethod>
class MacdMethod {
public:
  MacdMethod()
  : MacdMethod{12, 26, 9}
  {
  }

  MacdMethod(std::size_t short_period,
             std::size_t long_period,
             std::size_t signal_period)
  : MacdMethod{TSourceMethod{}, short_period, long_period, signal_period}
  {
  }

  MacdMethod(TSourceMethod source,
             std::size_t short_period,
             std::size_t long_period,
             std::size_t signal_period)
  : MacdMethod{std::move(source),
               ValueMethod{static_cast<double>(short_period)},
               ValueMethod{static_cast<double>(long_period)},
               ValueMethod{static_cast<double>(signal_period)}}
  {
  }

  MacdMethod(TSourceMethod source,
             TParameterMethod short_period,
             TParameterMethod long_period,
             TParameterMethod signal_period)
    requires(!std::is_arithmetic_v<TParameterMethod>)
  : source_{std::move(source)}
  , short_period_{std::move(short_period)}
  , long_period_{std::move(long_period)}
  , signal_period_{std::move(signal_period)}
  {
  }

  auto operator==(const MacdMethod& other) const noexcept -> bool = default;

  auto source(this const MacdMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this MacdMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto short_period(this const MacdMethod& self) noexcept
   -> const TParameterMethod&
  {
    return self.short_period_;
  }

  void short_period(this MacdMethod& self, TParameterMethod period) noexcept
  {
    self.short_period_ = period;
  }

  auto fast_period(this const MacdMethod& self) noexcept
   -> const TParameterMethod&
  {
    return self.short_period_;
  }

  void fast_period(this MacdMethod& self, TParameterMethod period) noexcept
  {
    self.short_period_ = period;
  }

  auto long_period(this const MacdMethod& self) noexcept
   -> const TParameterMethod&
  {
    return self.long_period_;
  }

  void long_period(this MacdMethod& self, TParameterMethod period) noexcept
  {
    self.long_period_ = period;
  }

  auto slow_period(this const MacdMethod& self) noexcept
   -> const TParameterMethod&
  {
    return self.long_period_;
  }

  void slow_period(this MacdMethod& self, TParameterMethod period) noexcept
  {
    self.long_period_ = period;
  }

  auto signal_period(this const MacdMethod& self) noexcept
   -> const TParameterMethod&
  {
    return self.signal_period_;
  }

  void signal_period(this MacdMethod& self, TParameterMethod period) noexcept
  {
    self.signal_period_ = period;
  }

private:
  TSourceMethod source_;
  TParameterMethod short_period_;
  TParameterMethod long_period_;
  TParameterMethod signal_period_;
};

} // namespace pludux
