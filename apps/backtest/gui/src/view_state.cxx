module;

#include <string>
#include <utility>

export module pludux.apps.backtest.view_state;

import pludux.backtest;
import pludux.apps.backtest.portfolio_backtest_setup_selections;

export namespace pludux::apps {

class ViewState {
public:
  ViewState() = default;

  ViewState(
   std::string imgui_ini_settings,
   backtest::PortfolioStoreHandle selected_portfolio_handle,
   PortfolioBacktestSetupSelections portfolio_backtest_setup_selections,
   backtest::BacktestStoreHandle selected_backtest_handle)
  : imgui_ini_settings_{std::move(imgui_ini_settings)}
  , selected_portfolio_handle_{selected_portfolio_handle}
  , portfolio_backtest_setup_selections_{std::move(
     portfolio_backtest_setup_selections)}
  , selected_backtest_handle_{selected_backtest_handle}
  {
  }

  auto operator==(const ViewState& other) const -> bool
  {
    return imgui_ini_settings_ == other.imgui_ini_settings_ &&
           selected_portfolio_handle_ == other.selected_portfolio_handle_ &&
           portfolio_backtest_setup_selections_ ==
            other.portfolio_backtest_setup_selections_ &&
           selected_backtest_handle_ == other.selected_backtest_handle_;
  }

  auto imgui_ini_settings(this const ViewState& self) noexcept
   -> const std::string&
  {
    return self.imgui_ini_settings_;
  }

  void imgui_ini_settings(this ViewState& self, std::string settings) noexcept
  {
    self.imgui_ini_settings_ = std::move(settings);
  }

  auto selected_portfolio_handle(this const ViewState& self) noexcept
   -> backtest::PortfolioStoreHandle
  {
    return self.selected_portfolio_handle_;
  }

  void selected_portfolio_handle(this ViewState& self,
                                 backtest::PortfolioStoreHandle handle) noexcept
  {
    self.selected_portfolio_handle_ = handle;
  }

  auto portfolio_backtest_setup_selections(this const ViewState& self) noexcept
   -> const PortfolioBacktestSetupSelections&
  {
    return self.portfolio_backtest_setup_selections_;
  }

  auto portfolio_backtest_setup_selections(this ViewState& self) noexcept
   -> PortfolioBacktestSetupSelections&
  {
    return self.portfolio_backtest_setup_selections_;
  }

  auto selected_backtest_handle(this const ViewState& self) noexcept
   -> backtest::BacktestStoreHandle
  {
    return self.selected_backtest_handle_;
  }

  void selected_backtest_handle(this ViewState& self,
                                backtest::BacktestStoreHandle handle) noexcept
  {
    self.selected_backtest_handle_ = handle;
  }

private:
  std::string imgui_ini_settings_;
  backtest::PortfolioStoreHandle selected_portfolio_handle_;
  PortfolioBacktestSetupSelections portfolio_backtest_setup_selections_;
  backtest::BacktestStoreHandle selected_backtest_handle_;
};

} // namespace pludux::apps
