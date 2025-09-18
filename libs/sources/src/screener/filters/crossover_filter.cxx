module;

#include <vector>

export module pludux.screener.crossover_filter;

import pludux.asset_snapshot;
import pludux.screener.screener_filter;
import pludux.screener.screener_method;

export namespace pludux::screener {

class CrossoverFilter {
public:
  CrossoverFilter(ScreenerMethod signal, ScreenerMethod reference)
  : signal_{std::move(signal)}
  , reference_{std::move(reference)}
  {
  }

  auto operator==(const CrossoverFilter& other) const noexcept
   -> bool = default;

  auto operator()(AssetSnapshot asset_data) const -> bool
  {
    const auto signal_result = signal_(asset_data);
    const auto reference_result = reference_(asset_data);

    return signal_result[0] > reference_result[0] &&
           signal_result[1] <= reference_result[1];
  }

  auto signal() const noexcept -> const ScreenerMethod&
  {
    return signal_;
  }

  auto reference() const noexcept -> const ScreenerMethod&
  {
    return reference_;
  }

private:
  ScreenerMethod signal_;
  ScreenerMethod reference_;
};

} // namespace pludux::screener
