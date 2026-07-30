module;

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

export module pludux.backtest:plots.momentum_histogram_plot_method;

import :plots.plot_method_contextable;

export namespace pludux::backtest {

template<typename TPlotSourceMethod>
class MomentumHistogramPlotMethod {
public:
  MomentumHistogramPlotMethod(TPlotSourceMethod source)
  : MomentumHistogramPlotMethod(
     std::move(source), 0xFF9AA626, 0xFFDBDFB2, 0xFF5053EF, 0xFFD2CDFF)
  {
  }

  MomentumHistogramPlotMethod(TPlotSourceMethod source,
                              std::uint32_t positive_rising_color,
                              std::uint32_t positive_falling_color,
                              std::uint32_t negative_falling_color,
                              std::uint32_t negative_rising_color)
  : source_{std::move(source)}
  , positive_rising_color_{positive_rising_color}
  , positive_falling_color_{positive_falling_color}
  , negative_falling_color_{negative_falling_color}
  , negative_rising_color_{negative_rising_color}
  {
  }

  auto operator==(const MomentumHistogramPlotMethod&) const noexcept
   -> bool = default;

  void operator()(this const MomentumHistogramPlotMethod& self,
                  PlotMethodContextable auto context)
  {
    const auto& data = self.source_(context);

    auto bar_colors = std::vector<std::uint32_t>{};
    bar_colors.reserve(data.size());

    for(auto i = std::size_t{0}; i < data.size(); ++i) {
      const auto value = data[i];
      const auto has_previous =
       i > 0 && std::isfinite(data[i - 1]) && std::isfinite(value);
      if(value >= 0.0) {
        const auto is_rising = !has_previous || value >= data[i - 1];
        bar_colors.push_back(is_rising ? self.positive_rising_color_
                                       : self.positive_falling_color_);
      } else {
        const auto is_falling = !has_previous || value <= data[i - 1];
        bar_colors.push_back(is_falling ? self.negative_falling_color_
                                        : self.negative_rising_color_);
      }
    }

    context.render_plot_momentum_histogram(data, bar_colors);
  }

  auto source(this const MomentumHistogramPlotMethod& self) noexcept
   -> const TPlotSourceMethod&
  {
    return self.source_;
  }

  void source(this MomentumHistogramPlotMethod& self,
              TPlotSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto
  positive_rising_color(this const MomentumHistogramPlotMethod& self) noexcept
   -> std::uint32_t
  {
    return self.positive_rising_color_;
  }

  void positive_rising_color(this MomentumHistogramPlotMethod& self,
                             std::uint32_t color) noexcept
  {
    self.positive_rising_color_ = color;
  }

  auto
  positive_falling_color(this const MomentumHistogramPlotMethod& self) noexcept
   -> std::uint32_t
  {
    return self.positive_falling_color_;
  }

  void positive_falling_color(this MomentumHistogramPlotMethod& self,
                              std::uint32_t color) noexcept
  {
    self.positive_falling_color_ = color;
  }

  auto
  negative_falling_color(this const MomentumHistogramPlotMethod& self) noexcept
   -> std::uint32_t
  {
    return self.negative_falling_color_;
  }

  void negative_falling_color(this MomentumHistogramPlotMethod& self,
                              std::uint32_t color) noexcept
  {
    self.negative_falling_color_ = color;
  }

  auto
  negative_rising_color(this const MomentumHistogramPlotMethod& self) noexcept
   -> std::uint32_t
  {
    return self.negative_rising_color_;
  }

  void negative_rising_color(this MomentumHistogramPlotMethod& self,
                             std::uint32_t color) noexcept
  {
    self.negative_rising_color_ = color;
  }

private:
  TPlotSourceMethod source_;
  std::uint32_t positive_rising_color_;
  std::uint32_t positive_falling_color_;
  std::uint32_t negative_falling_color_;
  std::uint32_t negative_rising_color_;
};

} // namespace pludux::backtest
