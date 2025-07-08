module;

#include <cstddef>

export module pludux.screener.bb_method;

import pludux.asset_snapshot;
import pludux.series;
import pludux.screener.screener_method;

namespace pludux::screener {

export class BbMethod {
public:
  enum class MaType { sma, ema, wma, rma, hma };

  BbMethod(BbOutput output,
           MaType ma_type,
           ScreenerMethod input,
           std::size_t period,
           double stddev,
           std::size_t offset = 0)
  : output_{output}
  , ma_type_{ma_type}
  , input_{input}
  , offset_{offset}
  , period_{period}
  , stddev_{stddev}
  {
  }

  auto operator()(AssetSnapshot asset_data) const
   -> SubSeries<PolySeries<double>>
  {
    const auto bb = [&]() -> PolySeries<double> {
      switch(ma_type_) {
      case MaType::ema:
        return BbSeries{
         output_, EmaSeries{input_(asset_data), period_}, stddev_};
      case MaType::wma:
        return BbSeries{
         output_, WmaSeries{input_(asset_data), period_}, stddev_};
      case MaType::rma:
        return BbSeries{
         output_, RmaSeries{input_(asset_data), period_}, stddev_};
      case MaType::hma:
        return BbSeries{
         output_, HmaSeries{input_(asset_data), period_}, stddev_};
      case MaType::sma:
      default:
        return BbSeries{
         output_, SmaSeries{input_(asset_data), period_}, stddev_};
      }
    }();

    return SubSeries{PolySeries<double>{bb},
                     static_cast<std::ptrdiff_t>(offset_)};
  }

  auto operator==(const BbMethod& other) const noexcept -> bool = default;

  auto offset() const noexcept -> std::size_t
  {
    return offset_;
  }

  void offset(std::size_t offset) noexcept
  {
    offset_ = offset;
  }

  auto output() const noexcept -> BbOutput
  {
    return output_;
  }

  void output(BbOutput output) noexcept
  {
    output_ = output;
  }

  auto period() const noexcept -> std::size_t
  {
    return period_;
  }

  void period(std::size_t period) noexcept
  {
    period_ = period;
  }

  auto stddev() const noexcept -> double
  {
    return stddev_;
  }

  void stddev(double stddev) noexcept
  {
    stddev_ = stddev;
  }

  auto input() const noexcept -> ScreenerMethod
  {
    return input_;
  }

  void input(ScreenerMethod input) noexcept
  {
    input_ = input;
  }

  auto ma_type() const noexcept -> MaType
  {
    return ma_type_;
  }

  void ma_type(MaType ma_type) noexcept
  {
    ma_type_ = ma_type;
  }

private:
  BbOutput output_;
  MaType ma_type_;
  ScreenerMethod input_;
  std::size_t offset_;
  std::size_t period_;
  double stddev_;
};

} // namespace pludux::screener
