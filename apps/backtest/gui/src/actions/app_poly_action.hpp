#ifndef PLUDUX_APPS_BACKTEST_ACTIONS_APP_POLY_ACTION_HPP
#define PLUDUX_APPS_BACKTEST_ACTIONS_APP_POLY_ACTION_HPP

#include <functional>

#include "../app_state_data.hpp"

namespace pludux::apps {

using AppPolyAction = std::function<void(AppStateData&)>;

} // namespace pludux::apps

#endif
