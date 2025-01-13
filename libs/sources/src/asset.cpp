#include <string>
#include <utility>
#include <vector>

#include <pludux/asset.hpp>
#include <pludux/quote.hpp>

namespace pludux {

Asset::Asset(std::string symbol)
: symbol_{std::move(symbol)}
{
}

Asset::Asset(std::string symbol, const std::vector<Quote>& quotes)
: Asset{std::move(symbol)}
{
  for(const auto& quote : quotes) {
    add_quote(quote);
  }
}

auto Asset::symbol() const noexcept -> const std::string&
{
  return symbol_;
}

auto Asset::quotes() const noexcept -> const QuoteSeries&
{
  return quote_series_;
}

void Asset::add_quote(Quote quote)
{
  quote_series_.add_quote(quote);
}

} // namespace pludux
