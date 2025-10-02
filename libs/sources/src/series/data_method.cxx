module;

#include <algorithm>
#include <cmath>
#include <format>
#include <iterator>
#include <limits>
#include <string>
#include <variant>

export module pludux:series.data_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

export namespace pludux {

class DataMethod {
public:
  using ResultType = double;

  DataMethod() = default;

  explicit DataMethod(std::string field)
  : field_{std::move(field)}
  {
  }

  auto operator==(const DataMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    return asset_snapshot.data(self.field_);
  }

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }

  auto field(this const auto& self) -> const std::string&
  {
    return self.field_;
  }

  void field(this auto& self, std::string new_field)
  {
    self.field_ = std::move(new_field);
  }

private:
  std::string field_{};
};

} // namespace pludux
