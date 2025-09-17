module;

#include "app_state_data.hpp"
#include <functional>

export module pludux.apps.backtest.actions;

export import :change_strategy_json_action;
export import :load_asset_csv_action;
export import :load_backtests_setup_action;

export namespace pludux::apps {

using PolyAction = std::function<void(AppStateData&)>;

} // namespace pludux::apps
