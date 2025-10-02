module;

#include <vector>

export module pludux:screener.crossover_filter;

import :asset_snapshot;
import :screener.method_call_context;

import :screener.screener_filter;
import :screener.any_method;

export namespace pludux::screener {

class CrossoverFilter {
public:
  CrossoverFilter(AnyMethod signal, AnyMethod reference)
  : signal_{std::move(signal)}
  , reference_{std::move(reference)}
  {
  }

  auto operator==(const CrossoverFilter& other) const noexcept
   -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodCallContext<double> auto context) -> bool
  {
    const auto signal_current = self.signal_(asset_snapshot, context);
    const auto signal_prev = self.signal_(asset_snapshot[1], context);
    const auto reference_current = self.reference_(asset_snapshot, context);
    const auto reference_prev = self.reference_(asset_snapshot[1], context);

    return signal_current > reference_current && signal_prev <= reference_prev;
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

} // namespace pludux::screener
