module;

#include <algorithm>
#include <iterator>
#include <vector>

export module pludux:series.value_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

export namespace pludux {

class ValueMethod {
public:
  using ResultType = double;

  explicit ValueMethod(ResultType value)
  : value_{value}
  {
  }

  auto operator==(const ValueMethod& other) const noexcept -> bool = default;

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return self.value_;
  }

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output_name,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }

  auto value(this auto self) noexcept -> ResultType
  {
    return self.value_;
  }

  void value(this auto& self, ResultType new_value) noexcept
  {
    self.value_ = new_value;
  }

private:
  ResultType value_;
};

} // namespace pludux
