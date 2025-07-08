#include <pludux/screener/bb_method.hpp>

namespace pludux::screener {

BbMethod::BbMethod(BbOutput output,
                   MaType ma_type,
                   ScreenerMethod input,
                   std::size_t period,
                   double stddev,
                   std::size_t offset)
: output_{output}
, ma_type_{ma_type}
, input_{input}
, offset_{offset}
, period_{period}
, stddev_{stddev}
{
}

auto BbMethod::operator()(AssetSnapshot asset_data) const
 -> SubSeries<PolySeries<double>>
{
  const auto bb = [&]() -> PolySeries<double> {
    switch(ma_type_) {
    case MaType::ema:
      return BbSeries{output_, EmaSeries{input_(asset_data), period_}, stddev_};
    case MaType::wma:
      return BbSeries{output_, WmaSeries{input_(asset_data), period_}, stddev_};
    case MaType::rma:
      return BbSeries{output_, RmaSeries{input_(asset_data), period_}, stddev_};
    case MaType::hma:
      return BbSeries{output_, HmaSeries{input_(asset_data), period_}, stddev_};
    case MaType::sma:
    default:
      return BbSeries{output_, SmaSeries{input_(asset_data), period_}, stddev_};
    }
  }();

  return SubSeries{PolySeries<double>{bb},
                   static_cast<std::ptrdiff_t>(offset_)};
}

auto BbMethod::operator==(const BbMethod& other) const noexcept -> bool = default;


auto BbMethod::offset() const noexcept -> std::size_t
{
  return offset_;
}

void BbMethod::offset(std::size_t offset) noexcept
{
  offset_ = offset;
}

auto BbMethod::output() const noexcept -> BbOutput
{
  return output_;
}

void BbMethod::output(BbOutput output) noexcept
{
  output_ = output;
}

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