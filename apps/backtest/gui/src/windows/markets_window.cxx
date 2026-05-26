module;

#include <algorithm>
#include <cstring>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <utility>

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
  , selected_market_handle_opt_{}
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

  std::optional<backtest::MarketStoreHandle> selected_market_handle_opt_;
  std::shared_ptr<backtest::Market> editing_market_ptr_;

  void render_markets_list(this auto& self, WindowContext& context)
  {
    const auto& app_state = context.app_state();
    const auto& market_handles = app_state.get_market_handles();
    const auto backtest_ptr = app_state.selected_backtest_if_present();

    ImGui::BeginGroup();
    ImGui::BeginChild(
     "item view",
     ImVec2(
      0,
      -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us

    for(std::size_t i = 0; i < market_handles.size(); ++i) {
      const auto market_handle = market_handles[i];
      const auto& market = app_state.get_market(market_handle);

      ImGui::PushID(i);

      ImGui::SetNextItemAllowOverlap();
      auto is_selected =
       backtest_ptr && backtest_ptr->market_handle() == market_handle;
      const auto row_start = ImGui::GetCursorScreenPos();
      const auto row_width = ImGui::GetContentRegionAvail().x;
      const auto row_height = ImGui::GetFrameHeight();

      ImGui::Selectable("##market_row",
                        &is_selected,
                        ImGuiSelectableFlags_AllowOverlap,
                        ImVec2(row_width, row_height));

      ImGui::SetCursorScreenPos(row_start);
      ImGui::AlignTextToFramePadding();
      ImGui::TextUnformatted(market.name().c_str());

      const auto spacing = ImGui::GetStyle().ItemSpacing.x;
      const auto frame_padding_x = ImGui::GetStyle().FramePadding.x;
      const auto edit_width =
       ImGui::CalcTextSize("Edit").x + (2.0f * frame_padding_x);
      const auto delete_width =
       ImGui::CalcTextSize("Delete").x + (2.0f * frame_padding_x);
      const auto more_width =
       ImGui::CalcTextSize("More...").x + (2.0f * frame_padding_x);
      const auto buttons_width =
       edit_width + spacing + delete_width + spacing + more_width;
      const auto buttons_start_x = row_start.x + row_width - buttons_width;

      ImGui::SetCursorScreenPos(ImVec2(buttons_start_x, row_start.y));
      if(ImGui::Button("Edit")) {
        self.current_page_ = MarketPage::Edit;
        self.selected_market_handle_opt_ = market_handle;
        self.editing_market_ptr_ = std::make_shared<backtest::Market>(market);
      }

      ImGui::SameLine();

      if(ImGui::Button("Delete")) {
        context.push_action([market_handle](ApplicationState& app_state) {
          app_state.remove_market(market_handle);
        });
      }

      ImGui::SameLine();
      if(ImGui::Button("More...")) {
        ImGui::OpenPopup("market_menu_more");
      }

      if(ImGui::BeginPopup("market_menu_more")) {
        if(ImGui::MenuItem("Duplicate")) {
          context.push_action([market_handle](ApplicationState& app_state) {
            const auto& market = app_state.get_market(market_handle);
            auto duplicate_market = market;
            duplicate_market.name(market.name() + " Copy");
            app_state.add_market(std::move(duplicate_market));
          });
        }

        const auto move_up_disabled = i == 0;
        if(ImGui::MenuItem("Move Up", nullptr, false, !move_up_disabled)) {
          context.push_action(
           [from_index = i, to_index = i - 1](ApplicationState& app_state) {
             app_state.reorder_list_market(from_index, to_index);
           });
        }

        const auto move_down_disabled = i == market_handles.size() - 1;
        if(ImGui::MenuItem("Move Down", nullptr, false, !move_down_disabled)) {
          context.push_action(
           [from_index = i, to_index = i + 1](ApplicationState& app_state) {
             app_state.reorder_list_market(from_index, to_index);
           });
        }

        ImGui::EndPopup();
      }

      ImGui::PopID();
    }

    ImGui::EndChild();
    if(ImGui::Button("Add New Market")) {
      self.current_page_ = MarketPage::AddNew;

      self.selected_market_handle_opt_ = std::nullopt;
      self.editing_market_ptr_ = std::make_shared<backtest::Market>();
    }

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
};

} // namespace pludux::apps
