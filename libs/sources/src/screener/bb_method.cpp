#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/bb_method.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

BbMethod::BbMethod(OutputName output,
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

auto BbMethod::operator()(AssetSnapshot asset_data) const -> PolySeries<double>
{
  const auto bb = [&]() -> PolySeries<double> {
    switch(ma_type_) {
    case MaType::ema:
      return OutputByNameSeries{
       BbSeries{EmaSeries{input_(asset_data), period_}, stddev_}, output_};
    case MaType::wma:
      return OutputByNameSeries{
       BbSeries{WmaSeries{input_(asset_data), period_}, stddev_}, output_};
    case MaType::rma:
      return OutputByNameSeries{
       BbSeries{RmaSeries{input_(asset_data), period_}, stddev_}, output_};
    case MaType::hma:
      return OutputByNameSeries{
       BbSeries{HmaSeries{input_(asset_data), period_}, stddev_}, output_};
    case MaType::sma:
    default:
      return OutputByNameSeries{
       BbSeries{SmaSeries{input_(asset_data), period_}, stddev_}, output_};
    }
  }();

  return LookbackSeries{PolySeries<double>{bb},
                        static_cast<std::ptrdiff_t>(offset_)};
}

auto BbMethod::operator==(const BbMethod& other) const noexcept
 -> bool = default;

auto BbMethod::offset() const noexcept -> std::size_t
{
  return offset_;
}

void BbMethod::offset(std::size_t offset) noexcept
{
  offset_ = offset;
}

auto BbMethod::output() const noexcept -> OutputName
{
  return output_;
}

void BbMethod::output(OutputName output) noexcept
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