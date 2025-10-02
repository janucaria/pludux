module;

#include <vector>

export module pludux:screener.crossover_filter;

import :asset_snapshot;
import :series.method_call_context;

import :screener.screener_filter;
import :series.any_method;

export namespace pludux::screener {

class CrossoverFilter {
public:
  CrossoverFilter(series::AnyMethod signal, series::AnyMethod reference)
  : signal_{std::move(signal)}
  , reference_{std::move(reference)}
  {
  }

  auto operator==(const CrossoverFilter& other) const noexcept
   -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  series::MethodCallContext<double> auto context) -> bool
  {
    const auto signal_current = self.signal_(asset_snapshot, context);
    const auto signal_prev = self.signal_(asset_snapshot[1], context);
    const auto reference_current = self.reference_(asset_snapshot, context);
    const auto reference_prev = self.reference_(asset_snapshot[1], context);

    return signal_current > reference_current && signal_prev <= reference_prev;
  }

  auto signal(this const auto& self) noexcept -> const series::AnyMethod&
  {
    return self.signal_;
  }

  auto reference(this const auto& self) noexcept -> const series::AnyMethod&
  {
    return self.reference_;
  }

private:
  series::AnyMethod signal_;
  series::AnyMethod reference_;
};

} // namespace pludux::screener
