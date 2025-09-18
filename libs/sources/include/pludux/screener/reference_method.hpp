#ifndef PLUDUX_PLUDUX_SCREENER_REFERENCE_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_REFERENCE_METHOD_HPP

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include <pludux/screener/method_registry.hpp>
#include <pludux/series.hpp>

import pludux.asset_snapshot;

namespace pludux::screener {

class ReferenceMethod {
public:
  explicit ReferenceMethod(std::shared_ptr<const MethodRegistry> registry,
                           std::string name);

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>;

  auto operator==(const ReferenceMethod& other) const noexcept -> bool;

  auto registry() const noexcept -> std::shared_ptr<const MethodRegistry>;

  void registry(std::shared_ptr<const MethodRegistry> new_registry) noexcept;

  auto name() const noexcept -> const std::string&;

  void name(std::string new_name) noexcept;

private:
  std::shared_ptr<const MethodRegistry> registry_;
  std::string name_{};
};

} // namespace pludux::screener

#endif
