#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/bb_method.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

BbMethod::BbMethod(MaType ma_type,
                   ScreenerMethod input,
                   std::size_t period,
                   double stddev)
: ma_type_{ma_type}
, input_{input}
, period_{period}
, stddev_{stddev}
{
}

auto BbMethod::operator()(AssetSnapshot asset_data) const -> PolySeries<double>
{
  switch(ma_type_) {
  case MaType::ema:
    return BbSeries{EmaSeries{input_(asset_data), period_}, stddev_};
  case MaType::wma:
    return BbSeries{WmaSeries{input_(asset_data), period_}, stddev_};
  case MaType::rma:
    return BbSeries{RmaSeries{input_(asset_data), period_}, stddev_};
  case MaType::hma:
    return BbSeries{HmaSeries{input_(asset_data), period_}, stddev_};
  case MaType::sma:
  default:
    return BbSeries{SmaSeries{input_(asset_data), period_}, stddev_};
  }
}

auto BbMethod::operator==(const BbMethod& other) const noexcept
 -> bool = default;

auto BbMethod::period() const noexcept -> std::size_t
{
  return period_;
}

void BbMethod::period(std::size_t period) noexcept
{
  period_ = period;
}

auto BbMethod::stddev() const noexcept -> double
{
  return stddev_;
}

void BbMethod::stddev(double stddev) noexcept
{
  stddev_ = stddev;
}

auto BbMethod::input() const noexcept -> ScreenerMethod
{
  return input_;
}

void BbMethod::input(ScreenerMethod input) noexcept
{
  input_ = input;
}

auto BbMethod::ma_type() const noexcept -> MaType
{
  return ma_type_;
}

void BbMethod::ma_type(MaType ma_type) noexcept
{
  ma_type_ = ma_type;
}

} // namespace pludux::screener