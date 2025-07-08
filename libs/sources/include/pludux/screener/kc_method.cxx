module;

#include <cstddef>
#include <pludux/screener/screener_method.hpp>

export module pludux.screener.kc_method;

import pludux.series;
import pludux.asset_snapshot;

namespace pludux::screener {

export class KcMethod {
public:
  KcMethod(KcOutput output,
           ScreenerMethod ma,
           ScreenerMethod range,
           double multiplier,
           std::size_t offset = 0)
  : offset_{offset}
  , output_{output}
  , multiplier_{multiplier}
  , ma_{std::move(ma)}
  , range_{std::move(range)}
  {
  }

  auto operator()(AssetSnapshot asset_data) const
   -> SubSeries<PolySeries<double>>
  {
    const auto ma_series = ma_(asset_data);
    const auto range_series = range_(asset_data);

    const auto kc = KcSeries{output_, ma_series, range_series, multiplier_};
    return SubSeries{PolySeries<double>{kc},
                     static_cast<std::ptrdiff_t>(offset_)};
  }

  auto operator==(const KcMethod& other) const noexcept -> bool = default;

  auto offset() const noexcept -> std::size_t
  {
    return offset_;
  }

  void offset(std::size_t offset) noexcept
  {
    offset_ = offset;
  }

  auto output() const noexcept -> KcOutput
  {
    return output_;
  }

  void output(KcOutput output) noexcept
  {
    output_ = output;
  }

  auto multiplier() const noexcept -> double
  {
    return multiplier_;
  }

  void multiplier(double multiplier) noexcept
  {
    multiplier_ = multiplier;
  }

  auto ma() const noexcept -> ScreenerMethod
  {
    return ma_;
  }

  void ma(ScreenerMethod ma) noexcept
  {
    ma_ = std::move(ma);
  }

  auto range() const noexcept -> ScreenerMethod
  {
    return range_;
  }

  void range(ScreenerMethod range) noexcept
  {
    range_ = std::move(range);
  }

private:
  std::size_t offset_;
  KcOutput output_;
  double multiplier_;
  ScreenerMethod ma_;
  ScreenerMethod range_;
};

} // namespace pludux::screener

