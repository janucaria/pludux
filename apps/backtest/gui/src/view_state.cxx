module;

#include <string>
#include <utility>

export module pludux.apps.backtest.view_state;

import pludux.backtest;
import pludux.apps.backtest.portfolio_strategy_selections;

export namespace pludux::apps {

class ViewState {
public:
  ViewState() = default;

  ViewState(
   std::string imgui_ini_settings,
   backtest::PortfolioStoreHandle selected_portfolio_handle,
     PortfolioStrategySelections portfolio_strategy_selections,
      backtest::SystemStoreHandle selected_system_handle,
      backtest::StrategyStoreHandle selected_strategy_handle)
  : imgui_ini_settings_{std::move(imgui_ini_settings)}
  , selected_portfolio_handle_{selected_portfolio_handle}
    , portfolio_strategy_selections_{std::move(portfolio_strategy_selections)}
     , selected_system_handle_{selected_system_handle}
   , selected_strategy_handle_{selected_strategy_handle}
  {
  }

  auto operator==(const ViewState& other) const -> bool
  {
    return imgui_ini_settings_ == other.imgui_ini_settings_ &&
           selected_portfolio_handle_ == other.selected_portfolio_handle_ &&
             portfolio_strategy_selections_ == other.portfolio_strategy_selections_ &&
              selected_system_handle_ == other.selected_system_handle_ &&
           selected_strategy_handle_ == other.selected_strategy_handle_;
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

  auto portfolio_strategy_selections(this const ViewState& self) noexcept
   -> const PortfolioStrategySelections&
  {
    return self.portfolio_strategy_selections_;
  }

  auto portfolio_strategy_selections(this ViewState& self) noexcept
   -> PortfolioStrategySelections&
  {
    return self.portfolio_strategy_selections_;
  }

  auto selected_system_handle(this const ViewState& self) noexcept
   -> backtest::SystemStoreHandle
  {
    return self.selected_system_handle_;
  }

  void selected_system_handle(this ViewState& self,
                              backtest::SystemStoreHandle handle) noexcept
  {
    self.selected_system_handle_ = handle;
  }

  auto selected_strategy_handle(this const ViewState& self) noexcept
   -> backtest::StrategyStoreHandle
  {
    return self.selected_strategy_handle_;
  }

  void selected_strategy_handle(this ViewState& self,
                                backtest::StrategyStoreHandle handle) noexcept
  {
    self.selected_strategy_handle_ = handle;
  }

private:
  std::string imgui_ini_settings_;
  backtest::PortfolioStoreHandle selected_portfolio_handle_;
  PortfolioStrategySelections portfolio_strategy_selections_;
  backtest::SystemStoreHandle selected_system_handle_;
  backtest::StrategyStoreHandle selected_strategy_handle_;
};

} // namespace pludux::apps
