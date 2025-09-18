module;

#include <cstddef>

export module pludux:screener.bb_method;

import :asset_snapshot;
import :screener.screener_method;

export namespace pludux::screener {

class BbMethod {
public:
  enum class MaType { sma, ema, wma, rma, hma };

  BbMethod(MaType ma_type,
           ScreenerMethod input,
           std::size_t period,
           double stddev)
  : ma_type_{ma_type}
  , input_{input}
  , period_{period}
  , stddev_{stddev}
  {
  }

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>
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

  auto operator==(const BbMethod& other) const noexcept -> bool = default;

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
  MaType ma_type_;
  ScreenerMethod input_;
  std::size_t period_;
  double stddev_;
};

} // namespace pludux::screener