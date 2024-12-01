#ifndef PLUDUX_PLUDUX_SCREENER_FIELD_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_FIELD_METHOD_HPP

#include <string>
#include <vector>

#include <pludux/asset.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class FieldMethod {
public:
  FieldMethod(std::string field, int offset);

  auto run_one(const Asset& asset) const -> double;

  auto run_all(const Asset& asset) const -> Series;

  auto field() const -> const std::string&;

  auto offset() const -> int;

private:
  std::string field_{};
  int offset_{};
};

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_FIELD_METHOD_HPP
