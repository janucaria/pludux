module;

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <vector>

export module pludux.screener.changes_method;

import pludux.asset_snapshot;
import pludux.series;
import pludux.screener.screener_method;
import pludux.screener.ohlcv_method;

namespace pludux::screener {

export class ChangesMethod {
public:
  ChangesMethod()
  : ChangesMethod{CloseMethod{}, 0}
  {
  }

  explicit ChangesMethod(ScreenerMethod input, std::size_t offset = 0)
  : input_{input}
  , offset_{offset}
  {
  }

  auto operator()(AssetSnapshot asset_data) const
   -> SubSeries<PolySeries<double>>
  {
    const auto input_series = input_(asset_data);
    const auto result = ChangeSeries{input_series};
    return SubSeries{PolySeries<double>{result},
                     static_cast<std::ptrdiff_t>(offset_)};
  }

  auto operator==(const ChangesMethod& other) const noexcept -> bool = default;

  auto offset() const noexcept -> std::size_t
  {
    return offset_;
  }

  void offset(std::size_t offset) noexcept
  {
    offset_ = offset;
  }

  auto input() const noexcept -> const ScreenerMethod&
  {
    return input_;
  }

  void input(ScreenerMethod input) noexcept
  {
    input_ = std::move(input);
  }

private:
  ScreenerMethod input_;
  std::size_t offset_;
};

} // namespace pludux::screener
