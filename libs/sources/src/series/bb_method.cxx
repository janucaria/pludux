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

import :series.adaptive_ma_method;
import :series.ohlcv_method;
import :series.stddev_method;

export namespace pludux {

template<typename TMaSourceMethod = CloseMethod>
  requires requires { typename TMaSourceMethod::ResultType; }
class BbMethod {
public:
  using ResultType = typename TMaSourceMethod::ResultType;

  BbMethod()
  : BbMethod{20, 1.5}
  {
  }

  BbMethod(std::size_t period, double stddev)
  : BbMethod{MaMethodType::Sma, TMaSourceMethod{}, period, stddev}
  {
  }

  BbMethod(MaMethodType ma_type,
           TMaSourceMethod ma,
           std::size_t period,
           double stddev)
  : ma_method_{ma_type, std::move(ma), period}
  , stddev_{stddev}
  {
  }

  auto operator==(const BbMethod& other) const noexcept -> bool = default;

  auto operator()(this const BbMethod& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return self(asset_snapshot, SeriesOutput::MiddleBand, context);
  }

  auto operator()(this const BbMethod& self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept -> ResultType
  {
    const auto middle = self.ma_method_(asset_snapshot, context);
    const auto ma_source = self.ma_method_.source();
    const auto ma_period = self.ma_method_.period();
    const auto stddev_method = StddevMethod{ma_source, ma_period};

    const auto std_dev = stddev_method(asset_snapshot, context);
    const auto std_dev_scaled = std_dev * self.stddev_;

    switch(output) {
    case SeriesOutput::MiddleBand:
      return middle;
    case SeriesOutput::UpperBand:
      return middle + std_dev_scaled;
    case SeriesOutput::LowerBand:
      return middle - std_dev_scaled;
    default:
      return std::numeric_limits<ResultType>::quiet_NaN();
    }
  }

  auto ma_type(this const BbMethod& self) noexcept -> MaMethodType
  {
    return self.ma_method_.ma_type();
  }

  auto ma_type(this BbMethod& self, MaMethodType ma_type) noexcept
  {
    self.ma_method_.ma_type(ma_type);
  }

  auto ma_source(this const BbMethod& self) noexcept -> const TMaSourceMethod&
  {
    return self.ma_method_.source();
  }

  void ma_source(this BbMethod& self, TMaSourceMethod ma_source) noexcept
  {
    self.ma_method_.source(std::move(ma_source));
  }

  auto stddev(this const BbMethod& self) noexcept -> double
  {
    return self.stddev_;
  }

  void stddev(this BbMethod& self, double new_stddev) noexcept
  {
    self.stddev_ = new_stddev;
  }

  auto period(this const BbMethod& self) noexcept -> std::size_t
  {
    return self.ma_method_.period();
  }

  void period(this BbMethod& self, std::size_t new_period) noexcept
  {
    self.ma_method_.period(new_period);
  }

private:
  AdaptiveMaMethod<TMaSourceMethod> ma_method_;
  double stddev_;
};

} // namespace pludux
