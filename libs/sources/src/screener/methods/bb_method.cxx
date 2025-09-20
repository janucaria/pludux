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

  auto operator==(const BbMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self, AssetSnapshot asset_data)
   -> PolySeries<double>
  {
    switch(self.ma_type_) {
    case MaType::ema:
      return BbSeries{EmaSeries{self.input_(asset_data), self.period_},
                      self.stddev_};
    case MaType::wma:
      return BbSeries{WmaSeries{self.input_(asset_data), self.period_},
                      self.stddev_};
    case MaType::rma:
      return BbSeries{RmaSeries{self.input_(asset_data), self.period_},
                      self.stddev_};
    case MaType::hma:
      return BbSeries{HmaSeries{self.input_(asset_data), self.period_},
                      self.stddev_};
    case MaType::sma:
    default:
      return BbSeries{SmaSeries{self.input_(asset_data), self.period_},
                      self.stddev_};
    }
  }

  auto period(this const auto& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this auto& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

  auto stddev(this const auto& self) noexcept -> double
  {
    return self.stddev_;
  }

  void stddev(this auto& self, double stddev) noexcept
  {
    self.stddev_ = stddev;
  }

  auto input(this const auto& self) noexcept -> ScreenerMethod
  {
    return self.input_;
  }

  void input(this auto& self, ScreenerMethod input) noexcept
  {
    self.input_ = input;
  }

  auto ma_type(this const auto& self) noexcept -> MaType
  {
    return self.ma_type_;
  }

  void ma_type(this auto& self, MaType ma_type) noexcept
  {
    self.ma_type_ = ma_type;
  }

private:
  MaType ma_type_;
  ScreenerMethod input_;
  std::size_t period_;
  double stddev_;
};

} // namespace pludux::screener