module;

#include <vector>

export module pludux:screener.crossover_filter;

import :asset_snapshot;
import :screener.screener_filter;
import :screener.screener_method;

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

  auto operator()(this const auto& self, AssetSnapshot asset_data) -> bool
  {
    const auto signal_result = self.signal_(asset_data);
    const auto reference_result = self.reference_(asset_data);

    return signal_result[0] > reference_result[0] &&
           signal_result[1] <= reference_result[1];
  }

  auto signal(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.signal_;
  }

  auto reference(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.reference_;
  }

private:
  ScreenerMethod signal_;
  ScreenerMethod reference_;
};

} // namespace pludux::screener
