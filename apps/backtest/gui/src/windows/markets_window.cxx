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
import :window_context;

export namespace pludux::apps {

class MarketsWindow {
public:
  MarketsWindow()
  : current_page_(MarketPage::List)
  , selected_market_ptr_{nullptr}
  , editing_market_ptr_{nullptr}
  {
  }

  void render(this auto& self, WindowContext& context)
  {
    ImGui::Begin("Markets", nullptr);
    switch(self.current_page_) {
    case MarketPage::AddNew:
      self.render_add_new_market(context);
      break;
    case MarketPage::Edit:
      self.render_edit_market(context);
      break;
    case MarketPage::List:
    default:
      self.render_markets_list(context);
      break;
    }

    ImGui::End();
  }

private:
  enum class MarketPage { List, AddNew, Edit } current_page_;

  std::shared_ptr<backtest::Market> selected_market_ptr_;
  std::shared_ptr<backtest::Market> editing_market_ptr_;

  void render_markets_list(this auto& self, WindowContext& context)
  {
    const auto& backtest = context.app_state().selected_backtest();
    const auto& markets = context.markets();

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
        auto is_selected = backtest && backtest->market_ptr() == market;
        ImGui::Selectable(market_name.c_str(), &is_selected);

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
          context.push_action([i](ApplicationState& app_state) {
            app_state.remove_market_at_index(i);
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

  void render_add_new_market(this auto& self, WindowContext& context)
  {
    const auto& markets = context.markets();

    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Add New Market");
    ImGui::Separator();

    self.edit_market_form();

    ImGui::EndChild();
    if(ImGui::Button("Create Market")) {
      self.submit_market_changes(context);
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.reset();
    }

    ImGui::EndGroup();
  }

  void render_edit_market(this auto& self, WindowContext& context)
  {
    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Edit Market");
    ImGui::Separator();

    self.edit_market_form();

    ImGui::EndChild();

    const auto same_market =
     *(self.selected_market_ptr_) == *(self.editing_market_ptr_);

    if(ImGui::Button("OK")) {
      self.submit_market_changes(context);
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.reset();
    }

    ImGui::SameLine();
    ImGui::BeginDisabled(same_market);
    if(ImGui::Button("Apply")) {
      self.submit_market_changes(context);
    }
    ImGui::EndDisabled();

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
      auto min_order_quantity = self.editing_market_ptr_->min_order_quantity();
      ImGui::InputDouble("Minimum Order Quantity", &min_order_quantity);
      self.editing_market_ptr_->min_order_quantity(min_order_quantity);
    }
    {
      auto quantity_step = self.editing_market_ptr_->quantity_step();
      ImGui::InputDouble("Quantity Step", &quantity_step);
      self.editing_market_ptr_->quantity_step(quantity_step);
    }
  }

  void submit_market_changes(this auto& self, WindowContext& context)
  {
    context.push_action(
     [market_ptr = self.selected_market_ptr_,
      edit_market_ptr = self.editing_market_ptr_](ApplicationState& app_state) {
       if(edit_market_ptr->name().empty()) {
         edit_market_ptr->name("Unnamed");
       }

       if(market_ptr == nullptr) {
         app_state.add_market(
          std::make_shared<backtest::Market>(*edit_market_ptr));
         return;
       }

       const auto reset_backtests = !market_ptr->equal_rules(*edit_market_ptr);
       if(reset_backtests) {
         for(auto& backtest : app_state.backtests()) {
           if(backtest->market_ptr() == market_ptr) {
             backtest->reset();
           }
         }
       }

       *market_ptr = *edit_market_ptr;
     });
  }

  void reset(this MarketsWindow& self) noexcept
  {
    self.current_page_ = MarketPage::List;
    self.selected_market_ptr_ = nullptr;
    self.editing_market_ptr_ = nullptr;
  }
};

} // namespace pludux::apps
