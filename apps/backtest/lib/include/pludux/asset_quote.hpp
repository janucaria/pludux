#ifndef PLUDUX_PLUDUX_ASSET_QUOTE_HPP
#define PLUDUX_PLUDUX_ASSET_QUOTE_HPP

#include <pludux/asset_snapshot.hpp>
#include <pludux/quote_access.hpp>
#include <pludux/screener.hpp>

namespace pludux {

class AssetQuote {
public:
  AssetQuote(AssetSnapshot asset_data, const QuoteAccess& quote_access);

  auto time() const noexcept -> SubSeries<PolySeries<double>>;

  auto open() const noexcept -> SubSeries<PolySeries<double>>;

  auto high() const noexcept -> SubSeries<PolySeries<double>>;

  auto low() const noexcept -> SubSeries<PolySeries<double>>;

  auto close() const noexcept -> SubSeries<PolySeries<double>>;

  auto volume() const noexcept -> SubSeries<PolySeries<double>>;

  auto get_asset_size() const noexcept -> std::size_t;

  auto asset_snapshot() const noexcept -> const AssetSnapshot&;

private:
  AssetSnapshot asset_snapshot_;
  const QuoteAccess& quote_access_;
};

} // namespace pludux

#endif
