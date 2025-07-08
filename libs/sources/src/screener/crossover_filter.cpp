#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/crossover_filter.hpp>
#include <pludux/screener/screener_filter.hpp>
#include <pludux/screener/screener_method.hpp>

namespace pludux::screener {

CrossoverFilter::CrossoverFilter(ScreenerMethod signal,
                                 ScreenerMethod reference)
: signal_{std::move(signal)}
, reference_{std::move(reference)}
{
}

auto CrossoverFilter::operator()(AssetSnapshot asset_data) const -> bool
{
  const auto signal_result = signal_(asset_data);
  const auto reference_result = reference_(asset_data);

  return signal_result[0] > reference_result[0] &&
         signal_result[1] <= reference_result[1];
}

auto CrossoverFilter::operator==(const CrossoverFilter& other) const noexcept
 -> bool = default;

auto CrossoverFilter::signal() const noexcept -> const ScreenerMethod&
{
  return signal_;
}

auto CrossoverFilter::reference() const noexcept -> const ScreenerMethod&
{
  return reference_;
}

} // namespace pludux::screener
