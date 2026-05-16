module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.macd_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.ohlcv_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod>
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
  : source_{std::move(source)}
  , short_period_{short_period}
  , long_period_{long_period}
  , signal_period_{signal_period}
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

  auto short_period(this const MacdMethod& self) noexcept -> std::size_t
  {
    return self.short_period_;
  }

  void short_period(this MacdMethod& self, std::size_t period) noexcept
  {
    self.short_period_ = period;
  }

  auto fast_period(this const MacdMethod& self) noexcept -> std::size_t
  {
    return self.short_period_;
  }

  void fast_period(this MacdMethod& self, std::size_t period) noexcept
  {
    self.short_period_ = period;
  }

  auto long_period(this const MacdMethod& self) noexcept -> std::size_t
  {
    return self.long_period_;
  }

  void long_period(this MacdMethod& self, std::size_t period) noexcept
  {
    self.long_period_ = period;
  }

  auto slow_period(this const MacdMethod& self) noexcept -> std::size_t
  {
    return self.long_period_;
  }

  void slow_period(this MacdMethod& self, std::size_t period) noexcept
  {
    self.long_period_ = period;
  }

  auto signal_period(this const MacdMethod& self) noexcept -> std::size_t
  {
    return self.signal_period_;
  }

  void signal_period(this MacdMethod& self, std::size_t period) noexcept
  {
    self.signal_period_ = period;
  }

private:
  TSourceMethod source_;
  std::size_t short_period_;
  std::size_t long_period_;
  std::size_t signal_period_;
};

} // namespace pludux
