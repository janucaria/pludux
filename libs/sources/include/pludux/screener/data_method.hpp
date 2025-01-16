#ifndef PLUDUX_PLUDUX_SCREENER_DATA_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_DATA_METHOD_HPP

#include <cmath>
#include <string>
#include <vector>

#include <pludux/asset.hpp>
#include <pludux/asset_data_provider.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class DataMethod {
public:
  DataMethod(std::string field, std::size_t offset);

  auto operator()(const AssetDataProvider& asset_data) const -> PolySeries<double>;

  auto field() const -> const std::string&;

  auto offset() const -> int;

private:
  std::string field_{};
  std::size_t offset_{};
};

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_FIELD_METHOD_HPP
