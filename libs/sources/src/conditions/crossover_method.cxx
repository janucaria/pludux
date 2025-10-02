module;

#include <vector>

export module pludux:conditions.crossover_method;

import :asset_snapshot;
import :series.method_contextable;

import :conditions.any_condition_method;
import :series.any_series_method;

export namespace pludux {

class CrossoverMethod {
public:
  CrossoverMethod(AnySeriesMethod signal, AnySeriesMethod reference)
  : signal_{std::move(signal)}
  , reference_{std::move(reference)}
  {
  }

  auto operator==(const CrossoverMethod& other) const noexcept
   -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) -> bool
  {
    const auto signal_current = self.signal_(asset_snapshot, context);
    const auto signal_prev = self.signal_(asset_snapshot[1], context);
    const auto reference_current = self.reference_(asset_snapshot, context);
    const auto reference_prev = self.reference_(asset_snapshot[1], context);

    return signal_current > reference_current && signal_prev <= reference_prev;
  }

  auto signal(this const auto& self) noexcept -> const AnySeriesMethod&
  {
    return self.signal_;
  }

  auto reference(this const auto& self) noexcept -> const AnySeriesMethod&
  {
    return self.reference_;
  }

private:
  AnySeriesMethod signal_;
  AnySeriesMethod reference_;
};

} // namespace pludux
