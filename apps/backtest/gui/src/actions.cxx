module;

#include <functional>

export module pludux.apps.backtest:actions;

export import :actions.load_strategy_json_action;
export import :actions.load_asset_csv_action;
export import :actions.edit_strategy_action;

import :application_state;

export namespace pludux::apps {

using PolyAction = std::function<void(ApplicationState&)>;

} // namespace pludux::apps
