module;

#include <cmath>
#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

export module pludux:series.bb_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.ohlcv_method;
import :series.ma_method_type;

export namespace pludux {

template<typename TSourceMethod = CloseMethod>
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
  : source_{std::move(source)}
  , period_{period}
  , stddev_{stddev}
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

  auto period(this const BbMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this BbMethod& self, std::size_t new_period) noexcept
  {
    self.period_ = new_period;
  }

  auto stddev(this const BbMethod& self) noexcept -> double
  {
    return self.stddev_;
  }

  void stddev(this BbMethod& self, double new_stddev) noexcept
  {
    self.stddev_ = new_stddev;
  }

private:
  TSourceMethod source_;
  std::size_t period_;
  double stddev_;
  MaMethodType ma_method_type_;
};

} // namespace pludux
