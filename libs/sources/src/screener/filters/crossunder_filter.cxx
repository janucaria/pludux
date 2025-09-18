module;

#include <vector>

export module pludux:screener.crossunder_filter;

import :asset_snapshot;
import :screener.screener_filter;
import :screener.screener_method;

export namespace pludux::screener {

class CrossunderFilter {
public:
  CrossunderFilter(ScreenerMethod signal, ScreenerMethod reference)
  : signal_{std::move(signal)}
  , reference_{std::move(reference)}
  {
  }

  auto operator==(const CrossunderFilter& other) const noexcept
   -> bool = default;

  auto operator()(AssetSnapshot asset_data) const -> bool
  {
    const auto signal_result = signal_(asset_data);
    const auto reference_result = reference_(asset_data);

    return signal_result[0] < reference_result[0] &&
           signal_result[1] >= reference_result[1];
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

// ------------------------------------------------------------------------

} // namespace pludux::screener
