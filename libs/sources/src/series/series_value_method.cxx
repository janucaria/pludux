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
  SeriesValueMethod(std::string name)
  : name_{std::move(name)}
  {
  }

  auto operator==(const SeriesValueMethod& other) const noexcept
   -> bool = default;

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