module;

#include <algorithm>
#include <array>
#include <cstring>
#include <iterator>
#include <memory>
#include <ranges>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

export module pludux.apps.backtest:windows.brokers_window;

import pludux.backtest;
import :app_state;

export namespace pludux::apps {

class EditBrokerAction {
public:
  EditBrokerAction(std::shared_ptr<backtest::Broker> broker_ptr,
                   backtest::Broker broker)
  : broker_ptr_{std::move(broker_ptr)}
  , new_broker_{std::move(broker)}
  {
  }

  void operator()(this const EditBrokerAction& self, AppStateData& state)
  {
    const auto broker_ptr = self.broker_ptr_;
    *broker_ptr = self.new_broker_;

    for(auto& backtest : state.backtests) {
      if(backtest.broker_ptr() == broker_ptr) {
        backtest.reset();
      }
    }
  }

private:
  std::shared_ptr<backtest::Broker> broker_ptr_;
  backtest::Broker new_broker_;
};

class BrokersWindow {
public:
  BrokersWindow()
  : current_page_(BrokerPage::List)
  {
  }

  void render(this auto& self, AppState& app_state)
  {
    auto& state = app_state.state();

    ImGui::Begin("Brokers", nullptr);
    switch(self.current_page_) {
    case BrokerPage::AddNewBroker:
      self.render_add_new_broker(app_state);
      break;
    case BrokerPage::EditBroker:
      self.render_edit_broker(app_state);
      break;
    case BrokerPage::List:
    default:
      self.render_brokers_list(app_state);
      break;
    }

    ImGui::End();
  }

private:
  enum class BrokerPage { List, AddNewBroker, EditBroker } current_page_;

  std::shared_ptr<backtest::Broker> selected_broker_ptr_;
  std::shared_ptr<backtest::Broker> editing_broker_ptr_;

  void render_brokers_list(this auto& self, AppState& app_state)
  {
    const auto& state = app_state.state();
    const auto& brokers = state.brokers;

    ImGui::BeginGroup();
    ImGui::BeginChild(
     "item view",
     ImVec2(
      0,
      -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us

    if(!brokers.empty()) {
      for(auto i = 0; i < brokers.size(); ++i) {
        const auto broker = brokers[i];
        const auto& broker_name = broker->name();

        ImGui::PushID(i);

        ImGui::SetNextItemAllowOverlap();
        ImGui::Text("%s", broker_name.c_str());

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 100);
        if(ImGui::Button("Edit")) {
          self.current_page_ = BrokerPage::EditBroker;
          self.selected_broker_ptr_ = broker;
          self.editing_broker_ptr_ =
           std::make_shared<backtest::Broker>(*broker);
        }

        ImGui::SameLine();

        if(ImGui::Button("Delete")) {
          app_state.push_action([i](AppStateData& state) {
            auto& brokers = state.brokers;

            const auto it = std::next(brokers.begin(), i);
            auto& broker_ptr = *it;

            auto& backtests = state.backtests;
            for(auto j = 0; j < backtests.size(); ++j) {
              auto& backtest = backtests[j];
              if(backtest.broker_ptr()->name() == broker_ptr->name()) {
                backtests.erase(std::next(backtests.begin(), j));

                if(state.selected_backtest_index > i ||
                   state.selected_backtest_index >= backtests.size()) {
                  --state.selected_backtest_index;
                }

                // Adjust the index since we removed an element
                --j;
              }
            }

            // Remove the broker from the vector
            state.brokers.erase(it);
          });
        }

        ImGui::PopID();
      }
    }
    ImGui::EndChild();
    if(ImGui::Button("Add New Broker")) {
      self.current_page_ = BrokerPage::AddNewBroker;

      self.selected_broker_ptr_ = nullptr;
      self.editing_broker_ptr_ = std::make_shared<backtest::Broker>("");
    }

    ImGui::EndGroup();
  }

  void render_add_new_broker(this auto& self, AppState& app_state)
  {
    const auto& state = app_state.state();
    const auto& brokers = state.brokers;

    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Add New Broker");
    ImGui::Separator();

    self.edit_broker_form();

    ImGui::EndChild();
    if(ImGui::Button("Create Broker")) {
      if(!self.editing_broker_ptr_->name().empty()) {
        // check if the broker name already exists
        auto new_broker_name = self.editing_broker_ptr_->name();

        const auto is_name_exists =
         std::ranges::any_of(brokers, [&](const auto& broker) {
           return broker->name() == new_broker_name;
         });

        if(is_name_exists) {
          app_state.push_action(
           [broker_name = std::move(new_broker_name)](AppStateData& state) {
             // TODO: Visual Studio 2026 have bug with include <format> causing
             // compile error
             const auto error_message =
              std::string("Broker name '") + broker_name + "' already exists.";
             state.alert_messages.push(error_message);
           });
        } else {
          app_state.push_action(
           [broker = std::move(self.editing_broker_ptr_)](AppStateData& state) {
             state.brokers.push_back(broker);
           });

          self.current_page_ = BrokerPage::List;
          self.editing_broker_ptr_ = nullptr;
        }
      } else {
        app_state.push_action([](AppStateData& state) {
          state.alert_messages.push("Please enter a broker name.");
        });
      }
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.current_page_ = BrokerPage::List;
    }

    ImGui::EndGroup();
  }

  void render_edit_broker(this auto& self, AppState& app_state)
  {
    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Edit Broker");
    ImGui::Separator();

    self.edit_broker_form();

    ImGui::EndChild();
    if(ImGui::Button("Edit")) {
      if(!self.editing_broker_ptr_->name().empty()) {
        app_state.push_action(EditBrokerAction{
         self.selected_broker_ptr_, std::move(*self.editing_broker_ptr_)});

        self.current_page_ = BrokerPage::List;
        self.selected_broker_ptr_ = nullptr;
        self.editing_broker_ptr_ = nullptr;
      } else {
        app_state.push_action([](AppStateData& state) {
          state.alert_messages.push("Broker name cannot be empty.");
        });
      }
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.current_page_ = BrokerPage::List;
      self.selected_broker_ptr_ = nullptr;
      self.editing_broker_ptr_ = nullptr;
    }

    ImGui::SameLine();
    if(ImGui::Button("Apply")) {
      if(!self.editing_broker_ptr_->name().empty()) {
        app_state.push_action(EditBrokerAction{self.selected_broker_ptr_,
                                               *self.editing_broker_ptr_});
      } else {
        app_state.push_action([](AppStateData& state) {
          state.alert_messages.push("Broker name cannot be empty.");
        });
      }
    }

    ImGui::EndGroup();
  }

  void edit_broker_form(this BrokersWindow& self)
  {
    {
      auto broker_name = self.editing_broker_ptr_->name();
      ImGui::InputText("Name", &broker_name);
      self.editing_broker_ptr_->name(broker_name);
    }
    {
      ImGui::SeparatorText("Fees");
      auto fees = self.editing_broker_ptr_->fees();
      if(fees.empty()) {
        ImGui::Text("No fees added.");
      } else {
        for(auto i = 0; i < fees.size(); ++i) {
          auto& fee = fees[i];
          ImGui::PushID(i);

          {
            auto fee_name = fee.name();
            ImGui::InputText("Fee Name", &fee_name);
            fee.name(std::move(fee_name));
          }
          {
            auto fee_value = fee.value();
            ImGui::InputDouble("Fee Value", &fee_value, 0.01, 1.0, "%.4f");
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

            if(ImGui::BeginCombo("Fee Type",
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

            if(ImGui::BeginCombo(
                "Fee Position",
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

            if(ImGui::BeginCombo(
                "Apply On", fee_trigger_names.at(fee.fee_trigger()).c_str())) {
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

          if(ImGui::Button("Remove")) {
            fees.erase(std::next(fees.begin(), i));
          }
          ImGui::PopID();

          ImGui::Separator();
        }
      }

      ImGui::Text("");

      if(ImGui::Button("Add Fee")) {
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
};

} // namespace pludux::apps
