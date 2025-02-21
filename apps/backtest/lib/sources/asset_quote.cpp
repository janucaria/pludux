#include <pludux/asset_snapshot.hpp>
#include <pludux/quote_access.hpp>

#include <pludux/asset_quote.hpp>

namespace pludux {

AssetQuote::AssetQuote(AssetSnapshot asset_snapshot,
                       const QuoteAccess& quote_access)
: asset_snapshot_{asset_snapshot}
, quote_access_{quote_access}
{
}

auto AssetQuote::time() const noexcept -> SubSeries<PolySeries<double>>
{
  return quote_access_.time(asset_snapshot_);
}

auto AssetQuote::open() const noexcept -> SubSeries<PolySeries<double>>
{
  return quote_access_.open(asset_snapshot_);
}

auto AssetQuote::high() const noexcept -> SubSeries<PolySeries<double>>
{
  return quote_access_.high(asset_snapshot_);
}

auto AssetQuote::low() const noexcept -> SubSeries<PolySeries<double>>
{
  return quote_access_.low(asset_snapshot_);
}

auto AssetQuote::close() const noexcept -> SubSeries<PolySeries<double>>
{
  return quote_access_.close(asset_snapshot_);
}

auto AssetQuote::volume() const noexcept -> SubSeries<PolySeries<double>>
{
  return quote_access_.volume(asset_snapshot_);
}

auto AssetQuote::get_asset_size() const noexcept -> std::size_t
{
  return asset_snapshot().size();
}

auto AssetQuote::asset_snapshot() const noexcept -> const AssetSnapshot&
{
  return asset_snapshot_;
}

} // namespace pludux
