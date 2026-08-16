module;

// FIXME: Work around MSVC modules import failure in <stop_token> when this
// aggregate module re-exports pludux-backed window modules.
#include <stop_token>

export module pludux.apps.backtest:windows;

export import :windows.assets_window;
export import :windows.watchlists_window;
export import :windows.models_window;
export import :windows.markets_window;
export import :windows.brokers_window;
export import :windows.profiles_window;
export import :windows.strategies_window;
export import :windows.backtest_overview_window;
export import :windows.systems_window;
export import :windows.portfolios_window;
export import :windows.dockspace_window;
export import :windows.backtest_chart_window;
export import :windows.trade_list_window;
