#ifndef PLUDUX_PLUDUX_ASSET_HPP
#define PLUDUX_PLUDUX_ASSET_HPP

#include <string>
#include <vector>

#include <pludux/quote.hpp>
#include <pludux/quote_series.hpp>

namespace pludux {

class Asset {
public:
  Asset(std::string symbol);

  Asset(std::string symbol, const std::vector<Quote>& quotes);

  auto symbol() const noexcept -> const std::string&;

  auto quotes() const noexcept -> const QuoteSeries&;

  void add_quote(Quote quote);

private:
  std::string symbol_;
  QuoteSeries quote_series_;
};

} // namespace pludux

#endif
