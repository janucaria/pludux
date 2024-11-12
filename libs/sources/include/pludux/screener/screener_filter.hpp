#ifndef PLUDUX_PLUDUX_SCREENER_SCREENER_FILTER_HPP
#define PLUDUX_PLUDUX_SCREENER_SCREENER_FILTER_HPP

#include <functional>
#include <vector>

#include <pludux/asset.hpp>

namespace pludux::screener {

using ScreenerFilter = std::function<auto(const Asset&) ->bool>;

}

#endif // PLUDUX_PLUDUX_SCREENER_SCREENER_FILTER_HPP
