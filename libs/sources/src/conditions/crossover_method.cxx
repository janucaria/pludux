module;

#include <vector>

export module pludux:conditions.crossover_method;

import :asset_snapshot;
import :method_contextable;

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

  auto operator()(this const CrossoverMethod& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) -> bool
  {
    const auto signal_current = self.signal_(asset_snapshot, context);
    const auto signal_prev = self.signal_(asset_snapshot[1], context);
    const auto reference_current = self.reference_(asset_snapshot, context);
    const auto reference_prev = self.reference_(asset_snapshot[1], context);

    return signal_current > reference_current && signal_prev <= reference_prev;
  }

  auto signal(this const CrossoverMethod& self) noexcept
   -> const AnySeriesMethod&
  {
    return self.signal_;
  }

  void signal(this CrossoverMethod& self, AnySeriesMethod signal) noexcept
  {
    self.signal_ = std::move(signal);
  }

  auto reference(this const CrossoverMethod& self) noexcept
   -> const AnySeriesMethod&
  {
    return self.reference_;
  }

  void reference(this CrossoverMethod& self, AnySeriesMethod reference) noexcept
  {
    self.reference_ = std::move(reference);
  }

private:
  AnySeriesMethod signal_;
  AnySeriesMethod reference_;
};

} // namespace pludux
