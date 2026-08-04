module;

#include <algorithm>
#include <cstring>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "../ui/pludux_icons.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

export module pludux.apps.backtest:windows.markets_window;

import pludux.backtest;
import :window_context;
import :ui.widgets;

export namespace pludux::apps {

class MarketsWindow {
public:
  MarketsWindow()
  : current_page_(MarketPage::List)
  , selected_market_handle_opt_{}
  , editing_market_ptr_{nullptr}
  {
  }

  void render(this auto& self, WindowContext& context)
  {
    ImGui::Begin("Markets");
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

  void discard_draft(this MarketsWindow& self) noexcept
  {
    self.reset();
  }

private:
  enum class MarketPage { List, AddNew, Edit } current_page_;

  std::optional<backtest::MarketStoreHandle> selected_market_handle_opt_;
  std::shared_ptr<backtest::Market> editing_market_ptr_;
  ImGuiTextFilter market_filter_;

  void render_markets_list(this auto& self, WindowContext& context)
  {
    const auto& app_state = context.app_state();
    const auto& market_handles = app_state.get_market_handles();
    const auto backtest_ptr = app_state.selected_backtest_if_present();

    ImGui::BeginGroup();
    if(ImGui::Button(PLUDUX_ICON_ADD " New Market")) {
      self.current_page_ = MarketPage::AddNew;
      self.selected_market_handle_opt_ = std::nullopt;
      self.editing_market_ptr_ = std::make_shared<backtest::Market>();
    }
    ImGui::Spacing();
    ui::search_filter(self.market_filter_, "##markets_search");
    ImGui::BeginChild("item view", ImVec2(0, 0));

    auto visible_market_count = std::size_t{0};
    for(std::size_t i = 0; i < market_handles.size(); ++i) {
      const auto market_handle = market_handles[i];
      const auto& market = app_state.get_market(market_handle);
      if(!self.market_filter_.PassFilter(market.name().c_str())) {
        continue;
      }
      ++visible_market_count;

      ImGui::PushID(i);

      {
        const auto selected =
         backtest_ptr && backtest_ptr->market_handle() == market_handle;
        const auto has_draft =
         self.selected_market_handle_opt_ == market_handle &&
         self.editing_market_ptr_ && *self.editing_market_ptr_ != market;
        const auto display_name =
         has_draft ? market.name() + " (Unsaved)" : market.name();
        const auto action = ui::resource_row(
         display_name.c_str(), selected, i, market_handles.size());
        if(action == ui::ResourceRowAction::Edit) {
          self.current_page_ = MarketPage::Edit;
          if(self.selected_market_handle_opt_ != market_handle ||
             !self.editing_market_ptr_) {
            self.selected_market_handle_opt_ = market_handle;
            self.editing_market_ptr_ =
             std::make_shared<backtest::Market>(market);
          }
        } else if(action == ui::ResourceRowAction::Duplicate) {
          context.push_action([market_handle](ApplicationState& app_state) {
            const auto& value = app_state.get_market(market_handle);
            auto copy = value;
            copy.name(value.name() + " Copy");
            app_state.add_market(std::move(copy));
          });
        } else if(action == ui::ResourceRowAction::MoveUp) {
          context.push_action([from = i](ApplicationState& app_state) {
            app_state.reorder_list_market(from, from - 1);
          });
        } else if(action == ui::ResourceRowAction::MoveDown) {
          context.push_action([from = i](ApplicationState& app_state) {
            app_state.reorder_list_market(from, from + 1);
          });
        } else if(action == ui::ResourceRowAction::Delete) {
          context.push_action([market_handle](ApplicationState& app_state) {
            app_state.remove_market(market_handle);
          });
        }
        ImGui::PopID();
        continue;
      }
    }

    if(market_handles.empty()) {
      ImGui::TextDisabled("No markets yet. Add one to get started.");
    } else if(visible_market_count == 0) {
      ImGui::TextDisabled("No markets match this search.");
    }

    ImGui::EndChild();
    ImGui::EndGroup();
  }

  void render_add_new_market(this auto& self, WindowContext& context)
  {
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

    const auto selected_market_handle =
     self.selected_market_handle_opt_.value();
    const auto& selected_market =
     context.app_state().get_market(selected_market_handle);

    const auto same_market = selected_market == *(self.editing_market_ptr_);

    if(ImGui::Button("OK")) {
      self.submit_market_changes(context);
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.leave_editor();
    }

    ImGui::SameLine();
    const auto draft_action = ui::apply_reset_button(!same_market);
    if(draft_action == ui::DraftAction::Apply) {
      self.submit_market_changes(context);
    } else if(draft_action == ui::DraftAction::Reset) {
      self.editing_market_ptr_ =
       std::make_shared<backtest::Market>(selected_market);
    }

    ImGui::EndGroup();
  }

  void edit_market_form(this auto& self)
  {
    ui::form_section(
     "Market Details",
     "Define the order-size rules enforced by this market. A blank name is "
     "saved as 'Unnamed'.");
    {
      auto market_name = self.editing_market_ptr_->name();
      ui::field_label("Name");
      ImGui::InputTextWithHint("##market_name", "Unnamed", &market_name);
      self.editing_market_ptr_->name(market_name);
    }

    ui::form_section(
     "Order Quantity",
     "Orders smaller than the minimum are invalid. Valid quantities are "
     "rounded to increments of the quantity step.");
    {
      auto min_order_quantity = self.editing_market_ptr_->min_order_quantity();
      ui::field_label("Minimum quantity");
      ImGui::InputDouble(
       "##minimum_order_quantity", &min_order_quantity, 0.01, 1.0, "%.8f");
      self.editing_market_ptr_->min_order_quantity(min_order_quantity);
    }
    {
      auto quantity_step = self.editing_market_ptr_->quantity_step();
      ui::field_label("Quantity step");
      ImGui::InputDouble("##quantity_step", &quantity_step, 0.01, 1.0, "%.8f");
      self.editing_market_ptr_->quantity_step(quantity_step);
    }
  }

  void submit_market_changes(this auto& self, WindowContext& context)
  {
    context.push_action(
     [market_handle_opt = self.selected_market_handle_opt_,
      edit_market_ptr = self.editing_market_ptr_](ApplicationState& app_state) {
       if(edit_market_ptr->name().empty()) {
         edit_market_ptr->name("Unnamed");
       }

       if(!market_handle_opt) {
         app_state.add_market(*edit_market_ptr);
         return;
       }

       app_state.update_market(market_handle_opt.value(), *edit_market_ptr);
     });
  }

  void reset(this MarketsWindow& self) noexcept
  {
    self.current_page_ = MarketPage::List;
    self.selected_market_handle_opt_ = std::nullopt;
    self.editing_market_ptr_ = nullptr;
  }

  void leave_editor(this MarketsWindow& self) noexcept
  {
    self.current_page_ = MarketPage::List;
  }
};

} // namespace pludux::apps
