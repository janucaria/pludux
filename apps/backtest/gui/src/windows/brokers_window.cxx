module;

#include <algorithm>
#include <array>
#include <cstring>
#include <format>
#include <iterator>
#include <memory>
#include <ranges>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../ui/pludux_icons.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

export module pludux.apps.backtest:windows.brokers_window;

import pludux.backtest;
import :window_context;
import :ui.widgets;

export namespace pludux::apps {

class BrokersWindow {
public:
  BrokersWindow()
  : current_page_(BrokerPage::List)
  {
  }

  void render(this auto& self, WindowContext& context)
  {
    ImGui::Begin("Brokers");
    switch(self.current_page_) {
    case BrokerPage::AddNewBroker:
      self.render_add_new_broker(context);
      break;
    case BrokerPage::EditBroker:
      self.render_edit_broker(context);
      break;
    case BrokerPage::List:
    default:
      self.render_brokers_list(context);
      break;
    }

    ImGui::End();
  }

  void discard_draft(this BrokersWindow& self) noexcept
  {
    self.reset();
  }

private:
  enum class BrokerPage { List, AddNewBroker, EditBroker } current_page_;

  std::optional<backtest::BrokerStoreHandle> selected_broker_handle_opt_;
  std::shared_ptr<backtest::Broker> editing_broker_ptr_;
  ImGuiTextFilter broker_filter_;

  void render_brokers_list(this auto& self, WindowContext& context)
  {
    const auto& app_state = context.app_state();
    const auto& broker_handles = app_state.get_broker_handles();
    const auto portfolio_ptr = app_state.selected_portfolio_if_present();

    ImGui::BeginGroup();
    if(ImGui::Button(PLUDUX_ICON_ADD " New Broker")) {
      self.current_page_ = BrokerPage::AddNewBroker;
      self.selected_broker_handle_opt_ = std::nullopt;
      self.editing_broker_ptr_ = std::make_shared<backtest::Broker>("");
    }
    ImGui::Spacing();
    ui::search_filter(self.broker_filter_, "##brokers_search");
    ImGui::BeginChild("item view", ImVec2(0, 0));

    auto visible_broker_count = std::size_t{0};
    for(std::size_t i = 0; i < broker_handles.size(); ++i) {
      const auto broker_handle = broker_handles[i];
      const auto& broker = app_state.get_broker(broker_handle);
      if(!self.broker_filter_.PassFilter(broker.name().c_str())) {
        continue;
      }
      ++visible_broker_count;
      ImGui::PushID(i);

      {
        const auto selected =
         portfolio_ptr && portfolio_ptr->broker_handle() == broker_handle;
        const auto has_draft =
         self.selected_broker_handle_opt_ == broker_handle &&
         self.editing_broker_ptr_ && *self.editing_broker_ptr_ != broker;
        const auto display_name =
         has_draft ? broker.name() + " (Unsaved)" : broker.name();
        const auto action = ui::resource_row(
         display_name.c_str(), selected, i, broker_handles.size());
        if(action == ui::ResourceRowAction::Edit) {
          self.current_page_ = BrokerPage::EditBroker;
          if(self.selected_broker_handle_opt_ != broker_handle ||
             !self.editing_broker_ptr_) {
            self.selected_broker_handle_opt_ = broker_handle;
            self.editing_broker_ptr_ =
             std::make_shared<backtest::Broker>(broker);
          }
        } else if(action == ui::ResourceRowAction::Duplicate) {
          context.push_action([broker_handle](ApplicationState& app_state) {
            const auto& value = app_state.get_broker(broker_handle);
            auto copy = value;
            copy.name(value.name() + " Copy");
            app_state.add_broker(std::move(copy));
          });
        } else if(action == ui::ResourceRowAction::MoveUp) {
          context.push_action([from = i](ApplicationState& app_state) {
            app_state.reorder_list_broker(from, from - 1);
          });
        } else if(action == ui::ResourceRowAction::MoveDown) {
          context.push_action([from = i](ApplicationState& app_state) {
            app_state.reorder_list_broker(from, from + 1);
          });
        } else if(action == ui::ResourceRowAction::Delete) {
          context.push_action([broker_handle](ApplicationState& app_state) {
            app_state.remove_broker(broker_handle);
          });
        }
        ImGui::PopID();
        continue;
      }
    }

    if(broker_handles.empty()) {
      ImGui::TextDisabled("No brokers yet. Add one to get started.");
    } else if(visible_broker_count == 0) {
      ImGui::TextDisabled("No brokers match this search.");
    }

    ImGui::EndChild();
    ImGui::EndGroup();
  }

  void render_add_new_broker(this auto& self, WindowContext& context)
  {
    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Add New Broker");
    ImGui::Separator();

    self.edit_broker_form();

    ImGui::EndChild();
    if(ImGui::Button("Create Broker")) {
      self.submit_broker_changes(context);
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.reset();
    }

    ImGui::EndGroup();
  }

  void render_edit_broker(this auto& self, WindowContext& context)
  {
    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Edit Broker");
    ImGui::Separator();

    self.edit_broker_form();

    ImGui::EndChild();

    const auto selected_broker_handle =
     self.selected_broker_handle_opt_.value();
    const auto& selected_broker =
     context.app_state().get_broker(selected_broker_handle);
    const auto same_broker = selected_broker == *(self.editing_broker_ptr_);

    if(ImGui::Button("OK")) {
      self.submit_broker_changes(context);
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.leave_editor();
    }

    ImGui::SameLine();
    const auto draft_action = ui::apply_reset_button(!same_broker);
    if(draft_action == ui::DraftAction::Apply) {
      self.submit_broker_changes(context);
    } else if(draft_action == ui::DraftAction::Reset) {
      self.editing_broker_ptr_ =
       std::make_shared<backtest::Broker>(selected_broker);
    }

    ImGui::EndGroup();
  }

  void edit_broker_form(this BrokersWindow& self)
  {
    ui::form_section(
     "Broker Details",
     "Configure the transaction costs applied during simulation. A blank "
     "name is saved as 'Unnamed'.");
    {
      auto broker_name = self.editing_broker_ptr_->name();
      ui::field_label("Name");
      ImGui::InputTextWithHint("##broker_name", "Unnamed", &broker_name);
      self.editing_broker_ptr_->name(broker_name);
    }
    {
      ui::form_section(
       "Fee Rules",
       "Each rule can target a trade direction and a specific order event. "
       "All matching rules are added together.");
      auto fees = self.editing_broker_ptr_->fees();
      if(fees.empty()) {
        ImGui::TextDisabled("No fee rules. This broker currently has no "
                            "transaction costs.");
      } else {
        for(auto i = 0; i < fees.size(); ++i) {
          auto& fee = fees[i];
          ImGui::PushID(i);

          const auto fee_heading = std::format(
           "Fee {}: {}", i + 1, fee.name().empty() ? "Unnamed" : fee.name());
          ImGui::SeparatorText(fee_heading.c_str());

          {
            auto fee_name = fee.name();
            ui::field_label("Name");
            ImGui::InputTextWithHint(
             "##fee_name", "e.g. Commission", &fee_name);
            fee.name(std::move(fee_name));
          }
          {
            auto fee_value = fee.value();
            ui::field_label(
             "Value",
             "Percentage fees use this value as a percent of order notional; "
             "fixed fees use it as a currency amount.");
            ImGui::InputDouble("##fee_value", &fee_value, 0.01, 1.0, "%.4f");
            fee.value(fee_value);
          }
          {
            const auto fee_type_options = std::array{
             pludux::backtest::BrokerFee::FeeType::PercentageNotional,
             pludux::backtest::BrokerFee::FeeType::Fixed};

            const auto fee_type_names =
             std::unordered_map<pludux::backtest::BrokerFee::FeeType,
                                std::string>{
              {pludux::backtest::BrokerFee::FeeType::PercentageNotional,
               "Percentage"},
              {pludux::backtest::BrokerFee::FeeType::Fixed, "Fixed"}};

            ui::field_label(
             "Calculation",
             "Percentage scales with order value. Fixed charges the same "
             "amount for every matching event.");
            if(ImGui::BeginCombo("##fee_type",
                                 fee_type_names.at(fee.fee_type()).c_str())) {
              for(const auto& type_option : fee_type_options) {
                const auto is_selected = fee.fee_type() == type_option;

                if(ImGui::Selectable(fee_type_names.at(type_option).c_str(),
                                     is_selected)) {
                  fee.fee_type(type_option);
                }

                if(is_selected) {
                  ImGui::SetItemDefaultFocus();
                }
              }
              ImGui::EndCombo();
            }
          }
          {
            const auto fee_position_options =
             std::array{pludux::backtest::BrokerFee::FeePosition::Long,
                        pludux::backtest::BrokerFee::FeePosition::Short,
                        pludux::backtest::BrokerFee::FeePosition::LongAndShort};

            const auto fee_position_names =
             std::unordered_map<pludux::backtest::BrokerFee::FeePosition,
                                std::string>{
              {pludux::backtest::BrokerFee::FeePosition::Long, "Long"},
              {pludux::backtest::BrokerFee::FeePosition::Short, "Short"},
              {pludux::backtest::BrokerFee::FeePosition::LongAndShort,
               "Long and Short"}};

            ui::field_label(
             "Position side",
             "Choose whether this rule applies to long trades, short trades, "
             "or both.");
            if(ImGui::BeginCombo(
                "##fee_position",
                fee_position_names.at(fee.fee_position()).c_str())) {
              for(const auto& position_option : fee_position_options) {
                const auto is_selected = fee.fee_position() == position_option;

                if(ImGui::Selectable(
                    fee_position_names.at(position_option).c_str(),
                    is_selected)) {
                  fee.fee_position(position_option);
                }

                if(is_selected) {
                  ImGui::SetItemDefaultFocus();
                }
              }
              ImGui::EndCombo();
            }
          }
          {
            const auto fee_trigger_options =
             std::array{pludux::backtest::BrokerFee::FeeTrigger::Entry,
                        pludux::backtest::BrokerFee::FeeTrigger::Exit,
                        pludux::backtest::BrokerFee::FeeTrigger::Buy,
                        pludux::backtest::BrokerFee::FeeTrigger::Sell,
                        pludux::backtest::BrokerFee::FeeTrigger::All};

            const auto fee_trigger_names =
             std::unordered_map<pludux::backtest::BrokerFee::FeeTrigger,
                                std::string>{
              {pludux::backtest::BrokerFee::FeeTrigger::Entry, "Entry"},
              {pludux::backtest::BrokerFee::FeeTrigger::Exit, "Exit"},
              {pludux::backtest::BrokerFee::FeeTrigger::Buy, "Buy"},
              {pludux::backtest::BrokerFee::FeeTrigger::Sell, "Sell"},
              {pludux::backtest::BrokerFee::FeeTrigger::All, "All"}};

            ui::field_label(
             "Order event",
             "Entry and Exit follow trade lifecycle; Buy and Sell follow the "
             "actual order direction.");
            if(ImGui::BeginCombo(
                "##apply_on",
                fee_trigger_names.at(fee.fee_trigger()).c_str())) {
              for(const auto& trigger_option : fee_trigger_options) {
                const auto is_selected = fee.fee_trigger() == trigger_option;

                if(ImGui::Selectable(
                    fee_trigger_names.at(trigger_option).c_str(),
                    is_selected)) {
                  fee.fee_trigger(trigger_option);
                }

                if(is_selected) {
                  ImGui::SetItemDefaultFocus();
                }
              }
              ImGui::EndCombo();
            }
          }

          if(ImGui::Button(PLUDUX_ICON_DELETE " Remove Fee")) {
            fees.erase(std::next(fees.begin(), i));
            --i;
          }
          ImGui::PopID();
        }
      }

      ImGui::Spacing();
      if(ImGui::Button(PLUDUX_ICON_ADD " Add Fee Rule")) {
        fees.push_back(
         backtest::BrokerFee{"New Fee",
                             backtest::BrokerFee::FeeType::PercentageNotional,
                             backtest::BrokerFee::FeePosition::LongAndShort,
                             backtest::BrokerFee::FeeTrigger::All,
                             0.0});
      }

      self.editing_broker_ptr_->fees(fees);

      ImGui::Text("");
    }
  }

  void submit_broker_changes(this auto& self, WindowContext& context)
  {
    context.push_action(
     [broker_handle_opt = self.selected_broker_handle_opt_,
      edit_broker_ptr = self.editing_broker_ptr_](ApplicationState& app_state) {
       if(edit_broker_ptr->name().empty()) {
         edit_broker_ptr->name("Unnamed");
       }

       if(!broker_handle_opt) {
         app_state.add_broker(*edit_broker_ptr);
         return;
       }

       app_state.update_broker(broker_handle_opt.value(), *edit_broker_ptr);
     });
  }

  void reset(this BrokersWindow& self) noexcept
  {
    self.current_page_ = BrokerPage::List;
    self.selected_broker_handle_opt_ = std::nullopt;
    self.editing_broker_ptr_ = nullptr;
  }

  void leave_editor(this BrokersWindow& self) noexcept
  {
    self.current_page_ = BrokerPage::List;
  }
};

} // namespace pludux::apps
