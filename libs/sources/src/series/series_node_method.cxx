module;

#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <variant>

export module pludux:series.series_node_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

export namespace pludux {

class SeriesNodeMethod {
public:
  using ResultType = double;

  SeriesNodeMethod(std::string name)
  : name_{std::move(name)}
  {
  }

  auto operator==(const SeriesNodeMethod& other) const noexcept
   -> bool = default;

  auto operator()(this const SeriesNodeMethod& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> ResultType
  {
    if constexpr(std::is_same_v<std::monostate, decltype(context)>) {
      return std::numeric_limits<ResultType>::quiet_NaN();
    } else {
      return context.call_series_method(self.name(), asset_snapshot);
    }
  }

  auto operator()(this const SeriesNodeMethod& self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output_name,
                  MethodContextable auto context) noexcept -> ResultType
  {
    if constexpr(std::is_same_v<std::monostate, decltype(context)>) {
      return std::numeric_limits<ResultType>::quiet_NaN();
    } else {
      return context.call_series_method(
       self.name(), asset_snapshot, output_name);
    }
  }

  auto name(this const SeriesNodeMethod& self) noexcept
   -> const std::string&
  {
    return self.name_;
  }

  void name(this SeriesNodeMethod& self, std::string new_name) noexcept
  {
    self.name_ = std::move(new_name);
  }

private:
  std::string name_{};
};

} // namespace pludux