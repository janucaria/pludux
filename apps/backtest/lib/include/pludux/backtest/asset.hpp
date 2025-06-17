#ifndef PLUDUX_PLUDUX_BACKTEST_ASSET_HPP
#define PLUDUX_PLUDUX_BACKTEST_ASSET_HPP

#include <string>

#include <pludux/asset_history.hpp>
#include <pludux/asset_quote.hpp>
#include <pludux/quote_access.hpp>

namespace pludux::backtest {

class Asset {
public:
  Asset(std::string name, AssetHistory asset_history, QuoteAccess quote_access);

  auto name() const noexcept -> const std::string&;

  auto history() const noexcept -> const AssetHistory&;

  auto quote_access() const noexcept -> const QuoteAccess&;

  auto get_quote(std::size_t asset_index = 0) const noexcept -> AssetQuote;

private:
  std::string name_;
  AssetHistory asset_history_;
  QuoteAccess quote_access_;
};

} // namespace pludux::backtest

#endif