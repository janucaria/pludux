#ifndef PLUDUX_PLUDUX_SCREENER_VALUE_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_VALUE_METHOD_HPP

#include <vector>

#include <pludux/asset.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class ValueMethod {
public:
  explicit ValueMethod(double value);

  auto run_one(const Asset& asset) const -> double;

  auto run_all(const Asset& asset) const -> Series;

  auto value() const -> double;

private:
  double value_;
};

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_VALUE_METHOD_HPP
