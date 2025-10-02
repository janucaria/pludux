module;

#include <vector>

export module pludux:screener.crossunder_filter;

import :asset_snapshot;
import :series.method_contextable;

import :screener.screener_filter;
import :series.any_method;

export namespace pludux::screener {

class CrossunderFilter {
public:
  CrossunderFilter(AnyMethod signal, AnyMethod reference)
  : signal_{std::move(signal)}
  , reference_{std::move(reference)}
  {
  }

  auto operator==(const CrossunderFilter& other) const noexcept
   -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_data,
                  MethodContextable auto context) -> bool
  {
    const auto signal_current = self.signal_(asset_data, context);
    const auto signal_prev = self.signal_(asset_data[1], context);
    const auto reference_current = self.reference_(asset_data, context);
    const auto reference_prev = self.reference_(asset_data[1], context);

    return signal_current < reference_current && signal_prev >= reference_prev;
  }

  auto signal(this const auto& self) noexcept -> const AnyMethod&
  {
    return self.signal_;
  }

  auto reference(this const auto& self) noexcept -> const AnyMethod&
  {
    return self.reference_;
  }

private:
  AnyMethod signal_;
  AnyMethod reference_;
};

// ------------------------------------------------------------------------

} // namespace pludux::screener
