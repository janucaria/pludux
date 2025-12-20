module;

#include <algorithm>
#include <cstring>
#include <iterator>
#include <memory>
#include <string>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

export module pludux.apps.backtest:windows.markets_window;

import pludux.backtest;
import :app_state;

export namespace pludux::apps {

class EditMarketAction {
public:
  EditMarketAction(std::shared_ptr<backtest::Market> market_ptr,
                   backtest::Market market)
  : market_ptr_{std::move(market_ptr)}
  , new_market_{std::move(market)}
  {
  }

  void operator()(this const EditMarketAction& self, AppStateData& state)
  {
    const auto market_ptr = self.market_ptr_;
    *market_ptr = self.new_market_;

    for(auto& backtest : state.backtests) {
      if(backtest.market_ptr() == market_ptr) {
        backtest.reset();
      }
    }
  }

private:
  std::shared_ptr<backtest::Market> market_ptr_;
  backtest::Market new_market_;
};

class MarketsWindow {
public:
  MarketsWindow()
  : current_page_(MarketPage::List)
  , selected_market_ptr_{nullptr}
  , editing_market_ptr_{nullptr}
  {
  }

  void render(this auto& self, AppState& app_state)
  {
    auto& state = app_state.state();

    ImGui::Begin("Markets", nullptr);
    switch(self.current_page_) {
    case MarketPage::AddNew:
      self.render_add_new_market(app_state);
      break;
    case MarketPage::Edit:
      self.render_edit_market(app_state);
      break;
    case MarketPage::List:
    default:
      self.render_markets_list(app_state);
      break;
    }

    ImGui::End();
  }

private:
  enum class MarketPage { List, AddNew, Edit } current_page_;

  std::shared_ptr<backtest::Market> selected_market_ptr_;
  std::shared_ptr<backtest::Market> editing_market_ptr_;

  void render_markets_list(this auto& self, AppState& app_state)
  {
    const auto& state = app_state.state();
    const auto& markets = state.markets;

    ImGui::BeginGroup();
    ImGui::BeginChild(
     "item view",
     ImVec2(
      0,
      -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us

    if(!markets.empty()) {
      for(auto i = 0; i < markets.size(); ++i) {
        const auto market = markets[i];
        const auto& market_name = market->name();

        ImGui::PushID(i);

        ImGui::SetNextItemAllowOverlap();
        ImGui::Text("%s", market_name.c_str());

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 100);
        if(ImGui::Button("Edit")) {
          self.current_page_ = MarketPage::Edit;
          self.selected_market_ptr_ = market;
          self.editing_market_ptr_ =
           std::make_shared<backtest::Market>(*market);
        }

        ImGui::SameLine();

        if(ImGui::Button("Delete")) {
          app_state.push_action([i](AppStateData& state) {
            auto& markets = state.markets;

            const auto it = std::next(markets.begin(), i);
            auto& market_ptr = *it;

            auto& backtests = state.backtests;
            for(auto j = 0; j < backtests.size(); ++j) {
              auto& backtest = backtests[j];
              if(backtest.market_ptr()->name() == market_ptr->name()) {
                backtests.erase(std::next(backtests.begin(), j));

                if(state.selected_backtest_index > i ||
                   state.selected_backtest_index >= backtests.size()) {
                  --state.selected_backtest_index;
                }

                // Adjust the index since we removed an element
                --j;
              }
            }

            // Remove the market from the vector
            state.markets.erase(it);
          });
        }

        ImGui::PopID();
      }
    }
    ImGui::EndChild();
    if(ImGui::Button("Add New Market")) {
      self.current_page_ = MarketPage::AddNew;

      self.selected_market_ptr_ = nullptr;
      self.editing_market_ptr_ = std::make_shared<backtest::Market>();
    }

    ImGui::EndGroup();
  }

  void render_add_new_market(this auto& self, AppState& app_state)
  {
    const auto& state = app_state.state();
    const auto& markets = state.markets;

    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Add New Market");
    ImGui::Separator();

    self.edit_market_form();

    ImGui::EndChild();
    if(ImGui::Button("Create Market")) {
      if(!self.editing_market_ptr_->name().empty()) {
        // check if the market name already exists
        auto new_market_name = self.editing_market_ptr_->name();

        const auto is_name_exists =
         std::ranges::any_of(markets, [&](const auto& market) {
           return market->name() == new_market_name;
         });

        if(is_name_exists) {
          app_state.push_action(
           [market_name = std::move(new_market_name)](AppStateData& state) {
             // TODO: Visual Studio 2026 have bug with include <format> causing
             // compile error
             const auto error_message =
              std::string("Market name '") + market_name + "' already exists.";
             state.alert_messages.push(error_message);
           });
        } else {
          app_state.push_action(
           [market = std::move(self.editing_market_ptr_)](AppStateData& state) {
             state.markets.push_back(market);
           });

          self.current_page_ = MarketPage::List;
          self.editing_market_ptr_ = nullptr;
        }
      } else {
        app_state.push_action([](AppStateData& state) {
          state.alert_messages.push("Market name cannot be empty.");
        });
      }
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.current_page_ = MarketPage::List;
    }

    ImGui::EndGroup();
  }

  void render_edit_market(this auto& self, AppState& app_state)
  {
    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Edit Market");
    ImGui::Separator();

    self.edit_market_form();

    ImGui::EndChild();
    if(ImGui::Button("Edit")) {
      if(!self.editing_market_ptr_->name().empty()) {
        app_state.push_action(EditMarketAction{
         self.selected_market_ptr_, std::move(*self.editing_market_ptr_)});

        self.current_page_ = MarketPage::List;
        self.selected_market_ptr_ = nullptr;
        self.editing_market_ptr_ = nullptr;
      } else {
        app_state.push_action([](AppStateData& state) {
          state.alert_messages.push("Market name cannot be empty.");
        });
      }
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.current_page_ = MarketPage::List;
      self.selected_market_ptr_ = nullptr;
      self.editing_market_ptr_ = nullptr;
    }

    ImGui::SameLine();
    if(ImGui::Button("Apply")) {
      if(!self.editing_market_ptr_->name().empty()) {
        app_state.push_action(EditMarketAction{self.selected_market_ptr_,
                                               *self.editing_market_ptr_});
      } else {
        app_state.push_action([](AppStateData& state) {
          state.alert_messages.push("Market name cannot be empty.");
        });
      }
    }

    ImGui::EndGroup();
  }

  void edit_market_form(this auto& self)
  {
    {
      auto market_name = self.editing_market_ptr_->name();
      ImGui::InputText("Name", &market_name);
      self.editing_market_ptr_->name(market_name);
    }
    {
      auto min_order_qty = self.editing_market_ptr_->min_order_qty();
      ImGui::InputDouble("Minimum Order Quantity", &min_order_qty);
      self.editing_market_ptr_->min_order_qty(min_order_qty);
    }
    {
      auto qty_step = self.editing_market_ptr_->qty_step();
      ImGui::InputDouble("Quantity Step", &qty_step);
      self.editing_market_ptr_->qty_step(qty_step);
    }
  }
};

} // namespace pludux::apps
