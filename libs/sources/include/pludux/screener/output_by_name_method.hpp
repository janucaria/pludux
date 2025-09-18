#ifndef PLUDUX_PLUDUX_SCREENER_OUTPUT_BY_NAME_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_OUTPUT_BY_NAME_METHOD_HPP

#include <pludux/screener/screener_method.hpp>
#include <pludux/series.hpp>

import pludux.series;
import pludux.asset_snapshot;

namespace pludux::screener {

class OutputByNameMethod {
public:
  OutputByNameMethod(ScreenerMethod source, OutputName output_name);

  auto operator==(const OutputByNameMethod& other) const noexcept -> bool;

  auto operator()(this const OutputByNameMethod& self, AssetSnapshot asset_data)
   -> PolySeries<double>;

  auto source(this const OutputByNameMethod& self) noexcept
   -> const ScreenerMethod&;

  void source(this OutputByNameMethod& self, ScreenerMethod source) noexcept;

  auto output(this const OutputByNameMethod& self) noexcept -> OutputName;

  void output(this OutputByNameMethod& self, OutputName output_name) noexcept;

private:
  ScreenerMethod source_;
  OutputName output_name_;
};

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_FIELD_METHOD_HPP
