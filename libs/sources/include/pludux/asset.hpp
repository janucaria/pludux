#ifndef PLUDUX_PLUDUX_ASSET_HPP
#define PLUDUX_PLUDUX_ASSET_HPP

#include <string>
#include <vector>

#include "quote.hpp"

namespace pludux {

class Asset {
public:
  Asset(std::string symbol);

  Asset(std::string symbol, std::vector<Quote> quotes);

  auto symbol() const noexcept -> const std::string&;

  auto quotes() const noexcept -> const std::vector<Quote>&;

  void add_quote(Quote quote);

private:
  std::string symbol_;
  std::vector<Quote> quotes_;
};

} // namespace pludux

#endif
