module;

#include <vector>

export module pludux:conditions.crossunder_method;

import :asset_snapshot;
import :method_contextable;

import :conditions.any_condition_method;
import :series.any_series_method;

export namespace pludux {

class CrossunderMethod {
public:
  CrossunderMethod(AnySeriesMethod signal, AnySeriesMethod reference)
  : signal_{std::move(signal)}
  , reference_{std::move(reference)}
  {
  }

  auto operator==(const CrossunderMethod& other) const noexcept
   -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) -> bool
  {
    const auto prev_snapshot = asset_snapshot[1];

    const auto signal_current = self.signal_(asset_snapshot, context);
    const auto signal_prev = self.signal_(prev_snapshot, context);
    const auto reference_current = self.reference_(asset_snapshot, context);
    const auto reference_prev = self.reference_(prev_snapshot, context);

    return signal_current < reference_current && signal_prev >= reference_prev;
  }

  auto signal(this const auto& self) noexcept -> const AnySeriesMethod&
  {
    return self.signal_;
  }

  void signal(this auto& self, AnySeriesMethod signal) noexcept
  {
    self.signal_ = std::move(signal);
  }

  auto reference(this const auto& self) noexcept -> const AnySeriesMethod&
  {
    return self.reference_;
  }

  void reference(this auto& self, AnySeriesMethod reference) noexcept
  {
    self.reference_ = std::move(reference);
  }

private:
  AnySeriesMethod signal_;
  AnySeriesMethod reference_;
};

// ------------------------------------------------------------------------

} // namespace pludux
