#include <string>
#include <utility>
#include <vector>

#include <pludux/quote.hpp>
#include <pludux/asset.hpp>

namespace pludux {

Asset::Asset(std::string symbol)
: symbol_{std::move(symbol)}
{
}

auto Asset::symbol() const noexcept -> const std::string&
{
  return symbol_;
}

auto Asset::quotes() const noexcept -> const std::vector<Quote>&
{
  return quotes_;
}

void Asset::add_quote(Quote quote)
{
  quotes_.push_back(std::move(quote));
}

} // namespace pludux
