#ifndef PLUDUX_PLUDUX_SCREENER_DATA_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_DATA_METHOD_HPP

#include <cmath>
#include <string>
#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class DataMethod {
public:
  explicit DataMethod(std::string field, std::size_t offset = 0);

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>;

  auto operator==(const DataMethod& other) const noexcept -> bool;

  auto field() const -> const std::string&;

  auto offset() const -> int;

private:
  std::string field_{};
  std::size_t offset_{};
};

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_FIELD_METHOD_HPP
