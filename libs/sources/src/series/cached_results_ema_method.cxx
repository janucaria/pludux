module;

#include <cmath>
#include <concepts>
#include <cstddef>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

export module pludux:series.cached_results_ema_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.sma_method;
import :series.ema_method;
import :series.ohlcv_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod>
class CachedResultsEmaMethod {
public:
  using ResultType = typename TSourceMethod::ResultType;

  CachedResultsEmaMethod()
  : CachedResultsEmaMethod{20}
  {
  }

  explicit CachedResultsEmaMethod(std::size_t period)
  : CachedResultsEmaMethod{TSourceMethod{}, period}
  {
  }

  explicit CachedResultsEmaMethod(TSourceMethod source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  , cached_results_{std::make_shared<std::vector<ResultType>>()}
  , cached_last_index_{std::make_shared<std::ptrdiff_t>(-1)}
  {
  }

  auto operator==(const CachedResultsEmaMethod& other) const noexcept -> bool
  {
    const auto no_cached_comparison =
     period_ == other.period_ && source_ == other.source_ &&
     *(cached_last_index_) == *(other.cached_last_index_);

    if(!no_cached_comparison) {
      return false;
    }

    const auto& lhs_results = *(cached_results_);
    const auto& rhs_results = *(other.cached_results_);

    if(lhs_results.size() != rhs_results.size()) {
      return false;
    }

    for(std::size_t i = 0; i < lhs_results.size(); ++i) {
      if(lhs_results[i] != rhs_results[i]) {
        return false;
      }
    }

    return true;
  }

  auto operator()(this const CachedResultsEmaMethod& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> ResultType
  {
    auto result = std::numeric_limits<ResultType>::quiet_NaN();

    if constexpr(std::is_same_v<std::remove_cvref_t<decltype(context)>,
                                std::monostate>) {
      result = EmaMethod{self.source_, self.period_}(asset_snapshot, context);
    } else {
      const auto current_index = static_cast<std::ptrdiff_t>(context.index());
      const auto cached_last_index = *(self.cached_last_index_);

      // check for invalid cached result
      if(current_index < cached_last_index) {
        self.cached_results_->clear();
        *(self.cached_last_index_) = -1;
      }

      auto& cached_results = *(self.cached_results_);
      const auto results_size =
       static_cast<std::ptrdiff_t>(cached_results.size());

      const auto snapshot_size =
       static_cast<std::ptrdiff_t>(asset_snapshot.size());

      if(snapshot_size > results_size) {
        const auto alpha = 2.0 / (self.period_ + 1);

        const auto results_to_compute = snapshot_size - results_size;
        for(auto ii = results_to_compute; ii > 0; --ii) {
          const auto i = ii - 1;

          if(cached_results.size() < self.period_) {
            cached_results.push_back(
             std::numeric_limits<ResultType>::quiet_NaN());
            continue;
          }

          const auto prev_result_index = snapshot_size - 1 - i - 1;
          const auto prev_result = cached_results[prev_result_index];

          if(std::isnan(prev_result)) {
            result =
             SmaMethod{self.source_, self.period_}(asset_snapshot[i], context);
            cached_results.push_back(result);
            continue;
          }

          const auto source_value = self.source_(asset_snapshot[i], context);
          result = source_value * alpha + prev_result * (1 - alpha);
          cached_results.push_back(result);
        }

        result = cached_results.back();
      } else if(snapshot_size > 0) {
        const auto cached_index = snapshot_size - 1;
        result = cached_results[cached_index];
      }

      *(self.cached_last_index_) = current_index;
    }

    return result;
  }

  auto operator()(this const CachedResultsEmaMethod& self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }

  auto source(this const CachedResultsEmaMethod& self) noexcept
   -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this CachedResultsEmaMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const CachedResultsEmaMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this CachedResultsEmaMethod& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  TSourceMethod source_;
  std::size_t period_;

  // TODO: Use a better caching mechanism
  std::shared_ptr<std::vector<ResultType>> cached_results_;
  std::shared_ptr<std::ptrdiff_t> cached_last_index_;
};

} // namespace pludux
