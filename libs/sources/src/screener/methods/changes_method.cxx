module;

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <vector>

export module pludux:screener.changes_method;

import :asset_snapshot;
import :screener.screener_method;
import :screener.ohlcv_method;

export namespace pludux::screener {

class ChangesMethod {
public:
  ChangesMethod()
  : ChangesMethod{CloseMethod{}}
  {
  }

  explicit ChangesMethod(ScreenerMethod input)
  : input_{input}
  {
  }

  auto operator==(const ChangesMethod& other) const noexcept -> bool = default;

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>
  {
    const auto input_series = input_(asset_data);
    const auto result = ChangeSeries{input_series};
    return result;
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
};

} // namespace pludux::screener