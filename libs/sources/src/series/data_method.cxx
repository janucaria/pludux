module;

#include <string>
#include <utility>

export module pludux:series.data_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

export namespace pludux {

class DataMethod {
public:
  DataMethod() = default;

  explicit DataMethod(std::string field)
  : field_{std::move(field)}
  {
  }

  auto operator==(const DataMethod& other) const noexcept -> bool = default;

  auto field(this const DataMethod& self) -> const std::string&
  {
    return self.field_;
  }

  void field(this DataMethod& self, std::string new_field)
  {
    self.field_ = std::move(new_field);
  }

private:
  std::string field_{};
};

} // namespace pludux
