#include <utility>

#include <pludux/backtest/asset.hpp>

namespace pludux::backtest {

Asset::Asset(std::string name,
             AssetHistory asset_history,
             QuoteAccess quote_access)
: name_{std::move(name)}
, asset_history_{std::move(asset_history)}
, quote_access_{std::move(quote_access)}
{
}

auto Asset::name() const noexcept -> const std::string&
{
  return name_;
}

auto Asset::history() const noexcept -> const AssetHistory&
{
  return asset_history_;
}

auto Asset::quote_access() const noexcept -> const QuoteAccess&
{
  return quote_access_;
}

auto Asset::get_quote(std::size_t asset_index) const noexcept -> AssetQuote
{
  return AssetQuote{AssetSnapshot{asset_index, asset_history_}, quote_access_};
}

} // namespace pludux::backtest
