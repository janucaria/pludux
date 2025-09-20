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

  auto operator()(this const auto& self, AssetSnapshot asset_data)
   -> PolySeries<double>
  {
    const auto input_series = self.input_(asset_data);
    const auto result = ChangeSeries{input_series};
    return result;
  }

  auto input(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.input_;
  }

  void input(this auto& self, ScreenerMethod input) noexcept
  {
    self.input_ = std::move(input);
  }

private:
  ScreenerMethod input_;
};

} // namespace pludux::screener