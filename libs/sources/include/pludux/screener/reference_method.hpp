#ifndef PLUDUX_PLUDUX_SCREENER_REFERENCE_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_REFERENCE_METHOD_HPP

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/method_registry.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class ReferenceMethod {
public:
  explicit ReferenceMethod(std::shared_ptr<const MethodRegistry> registry,
                           std::string name,
                           std::size_t offset = 0);

  auto operator()(AssetSnapshot asset_data) const
   -> SubSeries<PolySeries<double>>;

  auto operator==(const ReferenceMethod& other) const noexcept -> bool;

  auto registry() const noexcept -> std::shared_ptr<const MethodRegistry>;

  void registry(std::shared_ptr<const MethodRegistry> new_registry) noexcept;

  auto name() const noexcept -> const std::string&;

  void name(std::string new_name) noexcept;

  auto offset() const noexcept -> std::size_t;

  void offset(std::size_t new_offset) noexcept;

private:
  std::shared_ptr<const MethodRegistry> registry_;
  std::string name_{};
  std::size_t offset_{};
};

} // namespace pludux::screener

#endif
