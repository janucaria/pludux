#include <algorithm>
#include <format>
#include <iterator>
#include <stdexcept>
#include <vector>

#include <pludux/asset_snapshot.hpp>

#include <pludux/screener/output_by_name_method.hpp>

namespace pludux::screener {

OutputByNameMethod::OutputByNameMethod(ScreenerMethod source,
                                       OutputName output_name)
: source_{std::move(source)}
, output_name_{output_name}
{
}

auto OutputByNameMethod::operator==(
 const OutputByNameMethod& other) const noexcept -> bool = default;

auto OutputByNameMethod::operator()(this const OutputByNameMethod& self,
                                    AssetSnapshot asset_data)
 -> PolySeries<double>
{
  return OutputByNameSeries{self.source_(asset_data), self.output_name_};
}

auto OutputByNameMethod::source(this const OutputByNameMethod& self) noexcept
 -> const ScreenerMethod&
{
  return self.source_;
}

void OutputByNameMethod::source(this OutputByNameMethod& self,
                                ScreenerMethod source) noexcept
{
  self.source_ = std::move(source);
}

auto OutputByNameMethod::output(this const OutputByNameMethod& self) noexcept
 -> OutputName
{
  return self.output_name_;
}

void OutputByNameMethod::output(this OutputByNameMethod& self,
                                OutputName output_name) noexcept
{
  self.output_name_ = std::move(output_name);
}

} // namespace pludux::screener