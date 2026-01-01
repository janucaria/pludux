module;

#include <algorithm>
#include <iterator>
#include <memory>
#include <optional>
#include <queue>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

export module pludux.apps.backtest:application_state;

import pludux.backtest;

export namespace pludux::apps {

class ApplicationState {
public:
  ApplicationState()
  : ApplicationState(-1,
                     std::queue<std::string>{},
                     std::vector<std::shared_ptr<Backtest>>{},
                     std::vector<std::shared_ptr<backtest::Asset>>{},
                     std::vector<std::shared_ptr<backtest::Strategy>>{},
                     std::vector<std::shared_ptr<backtest::Market>>{},
                     std::vector<std::shared_ptr<backtest::Broker>>{},
                     std::vector<std::shared_ptr<backtest::Profile>>{})
  {
  }

  ApplicationState(std::ptrdiff_t selected_backtest_index,
                   std::queue<std::string> alert_messages,
                   std::vector<std::shared_ptr<Backtest>> backtests,
                   std::vector<std::shared_ptr<backtest::Asset>> assets,
                   std::vector<std::shared_ptr<backtest::Strategy>> strategies,
                   std::vector<std::shared_ptr<backtest::Market>> markets,
                   std::vector<std::shared_ptr<backtest::Broker>> brokers,
                   std::vector<std::shared_ptr<backtest::Profile>> profiles)
  : selected_backtest_index_{selected_backtest_index}
  , alert_messages_{std::move(alert_messages)}
  , backtests_{std::move(backtests)}
  , assets_{std::move(assets)}
  , strategies_{std::move(strategies)}
  , markets_{std::move(markets)}
  , brokers_{std::move(brokers)}
  , profiles_{std::move(profiles)}
  {
  }

  auto alert_messages(this const ApplicationState& self) noexcept
   -> const std::queue<std::string>&
  {
    return self.alert_messages_;
  }

  auto alert(this ApplicationState& self, std::string message)
  {
    self.alert_messages_.push(std::move(message));
  }

  auto pop_alert_messages(this ApplicationState& self) -> std::string
  {
    if(self.alert_messages_.empty()) {
      return {};
    }

    const auto message = self.alert_messages_.front();
    self.alert_messages_.pop();
    return message;
  }

  auto backtests(this const ApplicationState& self) noexcept
   -> const std::vector<std::shared_ptr<Backtest>>&
  {
    return self.backtests_;
  }

  auto backtests(this ApplicationState& self) noexcept
   -> std::vector<std::shared_ptr<Backtest>>&
  {
    return self.backtests_;
  }

  void add_backtest(this ApplicationState& self,
                    std::shared_ptr<Backtest> backtest_ptr)
  {
    self.backtests_.push_back(std::move(backtest_ptr));
  }

  void replace_backtest(this ApplicationState& self,
                        std::shared_ptr<Backtest> old_backtest_ptr,
                        std::shared_ptr<Backtest> new_backtest_ptr)
  {
    auto& backtests = self.backtests_;
    std::ranges::replace_if(
     backtests,
     [&old_backtest_ptr](const std::shared_ptr<Backtest>& backtest) {
       return backtest == old_backtest_ptr;
     },
     std::move(new_backtest_ptr));
  }

  void remove_backtest_at_index(this ApplicationState& self, std::size_t index)
  {
    auto& backtests = self.backtests_;

    backtests.erase(std::next(backtests.begin(), index));
    if(self.selected_backtest_index_ > index ||
       self.selected_backtest_index_ >= backtests.size()) {
      --self.selected_backtest_index_;
    }
  }

  auto select_backtest(this ApplicationState& self,
                       std::shared_ptr<Backtest> backtest_ptr) -> bool
  {
    const auto& backtests = self.backtests_;
    const auto it = std::find(backtests.begin(), backtests.end(), backtest_ptr);
    if(it != backtests.end()) {
      self.selected_backtest_index_ = std::distance(backtests.begin(), it);

      return true;
    }

    return false;
  }

  auto selected_backtest(this const ApplicationState& self) noexcept
   -> std::shared_ptr<Backtest>
  {
    const auto& backtests = self.backtests_;
    const auto index = self.selected_backtest_index_;

    if(index < 0 || static_cast<std::size_t>(index) >= backtests.size()) {
      return nullptr;
    }

    return backtests[static_cast<std::size_t>(index)];
  }

  auto assets(this const ApplicationState& self) noexcept
   -> const std::vector<std::shared_ptr<backtest::Asset>>&
  {
    return self.assets_;
  }

  void add_asset(this ApplicationState& self,
                 std::shared_ptr<backtest::Asset> asset_ptr)
  {
    self.assets_.push_back(std::move(asset_ptr));
  }

  void remove_asset_at_index(this ApplicationState& self, std::size_t index)
  {
    const auto& assets = self.assets_;

    const auto it = std::next(assets.begin(), index);
    const auto& asset_ptr = *it;

    auto& backtests = self.backtests_;
    for(auto j = 0; j < backtests.size(); ++j) {
      auto& backtest = backtests[j];
      if(backtest->asset_ptr()->name() == asset_ptr->name()) {
        backtests.erase(std::next(backtests.begin(), j));

        if(self.selected_backtest_index_ > index ||
           self.selected_backtest_index_ >= backtests.size()) {
          --self.selected_backtest_index_;
        }

        // Adjust the index since we removed an element
        --j;
      }
    }
    // Remove the asset from the vector
    self.assets_.erase(it);
  }

  auto strategies(this const ApplicationState& self) noexcept
   -> const std::vector<std::shared_ptr<backtest::Strategy>>&
  {
    return self.strategies_;
  }

  void add_strategy(this ApplicationState& self,
                    std::shared_ptr<backtest::Strategy> strategy_ptr)
  {
    self.strategies_.push_back(std::move(strategy_ptr));
  }

  void remove_strategy_at_index(this ApplicationState& self, std::size_t index)
  {
    const auto& strategies = self.strategies_;

    const auto it = std::next(strategies.begin(), index);
    const auto& strategy_ptr = *it;

    auto& backtests = self.backtests_;
    for(auto j = 0; j < backtests.size(); ++j) {
      auto& backtest = backtests[j];
      if(backtest->strategy_ptr() == strategy_ptr) {
        backtests.erase(std::next(backtests.begin(), j));

        if(self.selected_backtest_index_ > index ||
           self.selected_backtest_index_ >= backtests.size()) {
          --self.selected_backtest_index_;
        }

        // Adjust the index since we removed an element
        --j;
      }
    }
    // Remove the strategy from the vector
    self.strategies_.erase(it);
  }

  auto markets(this const ApplicationState& self) noexcept
   -> const std::vector<std::shared_ptr<backtest::Market>>&
  {
    return self.markets_;
  }

  void add_market(this ApplicationState& self,
                  std::shared_ptr<backtest::Market> market_ptr)
  {
    self.markets_.push_back(std::move(market_ptr));
  }

  void remove_market_at_index(this ApplicationState& self, std::size_t index)
  {
    const auto& markets = self.markets_;

    const auto it = std::next(markets.begin(), index);
    const auto& market_ptr = *it;

    auto& backtests = self.backtests_;
    for(auto j = 0; j < backtests.size(); ++j) {
      auto& backtest = backtests[j];
      if(backtest->market_ptr() == market_ptr) {
        backtests.erase(std::next(backtests.begin(), j));

        if(self.selected_backtest_index_ > index ||
           self.selected_backtest_index_ >= backtests.size()) {
          --self.selected_backtest_index_;
        }

        // Adjust the index since we removed an element
        --j;
      }
    }
    // Remove the market from the vector
    self.markets_.erase(it);
  }

  auto brokers(this const ApplicationState& self) noexcept
   -> const std::vector<std::shared_ptr<backtest::Broker>>&
  {
    return self.brokers_;
  }

  void add_broker(this ApplicationState& self,
                  std::shared_ptr<backtest::Broker> broker_ptr)
  {
    self.brokers_.push_back(std::move(broker_ptr));
  }

  void remove_broker_at_index(this ApplicationState& self, std::size_t index)
  {
    const auto& brokers = self.brokers_;

    const auto it = std::next(brokers.begin(), index);
    const auto& broker_ptr = *it;

    auto& backtests = self.backtests_;
    for(auto j = 0; j < backtests.size(); ++j) {
      auto& backtest = backtests[j];
      if(backtest->broker_ptr() == broker_ptr) {
        backtests.erase(std::next(backtests.begin(), j));

        if(self.selected_backtest_index_ > index ||
           self.selected_backtest_index_ >= backtests.size()) {
          --self.selected_backtest_index_;
        }

        // Adjust the index since we removed an element
        --j;
      }
    }
    // Remove the broker from the vector
    self.brokers_.erase(it);
  }

  auto profiles(this const ApplicationState& self) noexcept
   -> const std::vector<std::shared_ptr<backtest::Profile>>&
  {
    return self.profiles_;
  }

  void add_profile(this ApplicationState& self,
                   std::shared_ptr<backtest::Profile> profile_ptr)
  {
    self.profiles_.push_back(std::move(profile_ptr));
  }

  void remove_profile_at_index(this ApplicationState& self, std::size_t index)
  {
    const auto& profiles = self.profiles_;

    const auto it = std::next(profiles.begin(), index);
    const auto& profile_ptr = *it;

    auto& backtests = self.backtests_;
    for(auto j = 0; j < backtests.size(); ++j) {
      auto& backtest = backtests[j];
      if(backtest->profile_ptr() == profile_ptr) {
        backtests.erase(std::next(backtests.begin(), j));

        if(self.selected_backtest_index_ > index ||
           self.selected_backtest_index_ >= backtests.size()) {
          --self.selected_backtest_index_;
        }

        // Adjust the index since we removed an element
        --j;
      }
    }
    // Remove the profile from the vector
    self.profiles_.erase(it);
  }

private:
  std::ptrdiff_t selected_backtest_index_{-1};

  std::queue<std::string> alert_messages_{};

  std::vector<std::shared_ptr<Backtest>> backtests_{};

  std::vector<std::shared_ptr<backtest::Asset>> assets_{};

  std::vector<std::shared_ptr<backtest::Strategy>> strategies_{};

  std::vector<std::shared_ptr<backtest::Market>> markets_{};

  std::vector<std::shared_ptr<backtest::Broker>> brokers_{};

  std::vector<std::shared_ptr<backtest::Profile>> profiles_{};
};

} // namespace pludux::apps
