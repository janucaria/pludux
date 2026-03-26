module;

#include <limits>
#include <string>
#include <utility>
#include <variant>

export module pludux:series.series_value_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

export namespace pludux {

class SeriesValueMethod {
public:
  using ResultType = double;

  SeriesValueMethod(std::string name)
  : name_{std::move(name)}
  {
  }

  auto operator==(const SeriesValueMethod& other) const noexcept
   -> bool = default;

  auto operator()(this const SeriesValueMethod& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> ResultType
  {
    if constexpr(std::is_same_v<std::monostate, decltype(context)>) {
      return std::numeric_limits<ResultType>::quiet_NaN();
    }

    const auto result_index = asset_snapshot.index();
    return context.get_series_result(self.name(), result_index);
  }

  auto operator()(this const SeriesValueMethod& self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output_name,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }

  auto name(this const SeriesValueMethod& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this SeriesValueMethod& self, std::string new_name) noexcept
  {
    self.name_ = std::move(new_name);
  }

private:
  std::string name_{};
};

} // namespace pludux