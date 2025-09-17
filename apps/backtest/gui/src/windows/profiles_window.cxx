module;

#include <format>
#include <iterator>

#include <imgui.h>

#include "../actions.hpp"

export module pludux.apps.backtest.windows:profiles_window;

import pludux.apps.backtest.app_state;

namespace pludux::apps {

export class ProfilesWindow {
public:
  ProfilesWindow()
  : current_page_(ProfilePage::List)
  , selected_profile_name_{}
  , input_name_{}
  , input_initial_capital_{100'000'000.0}
  , input_capital_risk_{0.01}
  {
  }

  void render(this auto& self, AppState& app_state)
  {
    auto& state = app_state.state();

    ImGui::Begin("Profiles", nullptr);
    switch(self.current_page_) {
    case ProfilePage::AddNewProfile:
      self.render_add_new_profile(app_state);
      break;
    case ProfilePage::EditProfile:
      self.render_edit_profile(app_state);
      break;
    case ProfilePage::List:
    default:
      self.render_profiles_list(app_state);
      break;
    }

    ImGui::End();
  }

private:
  enum class ProfilePage { List, AddNewProfile, EditProfile } current_page_;

  std::string selected_profile_name_;

  std::string input_name_;
  double input_initial_capital_;
  double input_capital_risk_;

  void render_profiles_list(this auto& self, AppState& app_state)
  {
    auto& state = app_state.state();
    auto& profiles = state.profiles;

    ImGui::BeginGroup();
    ImGui::BeginChild(
     "item view",
     ImVec2(
      0,
      -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us

    if(!profiles.empty()) {
      for(auto i = 0; i < profiles.size(); ++i) {
        const auto& profile = *profiles[i];
        const auto& profile_name = profile.name();

        ImGui::PushID(i);

        ImGui::SetNextItemAllowOverlap();
        ImGui::Text("%s", profile_name.c_str());

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 100);
        if(ImGui::Button("Edit")) {
          self.current_page_ = ProfilePage::EditProfile;
          self.selected_profile_name_ = profile_name;
          self.input_name_ = profile_name;
          self.input_initial_capital_ = profile.initial_capital();
          self.input_capital_risk_ = profile.capital_risk();
        }

        ImGui::SameLine();

        if(ImGui::Button("Delete")) {
          app_state.push_action([i](AppStateData& state) {
            auto& profiles = state.profiles;

            const auto it = std::next(profiles.begin(), i);
            auto& profile_ptr = *it;

            auto& backtests = state.backtests;
            for(auto j = 0; j < backtests.size(); ++j) {
              auto& backtest = backtests[j];
              if(backtest.profile_ptr()->name() == profile_ptr->name()) {
                backtests.erase(std::next(backtests.begin(), j));

                if(state.selected_backtest_index > i ||
                   state.selected_backtest_index >= backtests.size()) {
                  --state.selected_backtest_index;
                }

                // Adjust the index since we removed an element
                --j;
              }
            }

            // Remove the profile from the vector
            state.profiles.erase(it);
          });
        }

        ImGui::PopID();
      }
    }
    ImGui::EndChild();
    if(ImGui::Button("Add New Profile")) {
      self.current_page_ = ProfilePage::AddNewProfile;

      self.input_name_ = "";
      self.input_initial_capital_ = 100'000'000.0;
      self.input_capital_risk_ = 0.01;
    }

    ImGui::EndGroup();
  }

  void render_add_new_profile(this auto& self, AppState& app_state)
  {
    const auto& state = app_state.state();
    const auto& profiles = state.profiles;

    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Add New Profile");
    ImGui::Separator();

    {
      char name_buffer[64];
      std::strcpy(name_buffer, self.input_name_.c_str());
      ImGui::InputText("Name", name_buffer, sizeof(name_buffer));
      self.input_name_ = std::string{name_buffer};
    }

    ImGui::InputDouble(
     "Initial Capital", &self.input_initial_capital_, 100.0, 1000.0, "%.0f");

    {
      auto percentage = self.input_capital_risk_ * 100.0;
      ImGui::InputDouble("Capital Risk (%)", &percentage, 1.0, 10.0, "%.2f");
      self.input_capital_risk_ = percentage / 100.0;
    }

    ImGui::EndChild();
    if(ImGui::Button("Create Profile")) {
      if(!self.input_name_.empty()) {
        // check if the profile name already exists
        const auto is_name_exists = std::any_of(
         profiles.cbegin(), profiles.cend(), [&](const auto& profile) {
           return profile->name() == self.input_name_;
         });

        if(is_name_exists) {
          app_state.push_action([new_profile_name =
                                  self.input_name_](AppStateData& state) {
            const auto error_message =
             std::format("Profile name '{}' already exists.", new_profile_name);
            state.alert_messages.push(error_message);
          });
        } else {
          app_state.push_action(
           [name = self.input_name_,
            initial_capital = self.input_initial_capital_,
            capital_risk = self.input_capital_risk_](AppStateData& state) {
             auto profile = std::make_shared<backtest::Profile>(name);
             profile->initial_capital(initial_capital);
             profile->capital_risk(capital_risk);

             state.profiles.push_back(profile);
           });

          self.current_page_ = ProfilePage::List;
        }
      } else {
        app_state.push_action([](AppStateData& state) {
          state.alert_messages.push("Please enter a profile name.");
        });
      }
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.current_page_ = ProfilePage::List;
    }

    ImGui::EndGroup();
  }

  void render_edit_profile(this auto& self, AppState& app_state)
  {
    const auto& state = app_state.state();
    const auto& profiles = state.profiles;

    const auto find_it =
     std::find_if(profiles.cbegin(), profiles.cend(), [&](const auto& profile) {
       return profile->name() == self.selected_profile_name_;
     });

    if(find_it == profiles.cend()) {
      app_state.push_action(
       [name = self.selected_profile_name_](AppStateData& state) {
         const auto error_message =
          std::format("Profile '{}' not found for Edit.", name);
         state.alert_messages.push(error_message);
       });
      self.current_page_ = ProfilePage::List;
      return;
    }

    auto profile = *find_it;

    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Edit Profile");
    ImGui::Separator();

    {
      char name_buffer[64];
      std::strcpy(name_buffer, self.input_name_.c_str());
      ImGui::InputText("Name", name_buffer, sizeof(name_buffer));
      self.input_name_ = std::string{name_buffer};
    }

    ImGui::InputDouble(
     "Initial Capital", &self.input_initial_capital_, 100.0, 1000.0, "%.0f");

    {
      auto percentage = self.input_capital_risk_ * 100.0;
      ImGui::InputDouble("Capital Risk (%)", &percentage, 1.0, 10.0, "%.2f");
      self.input_capital_risk_ = percentage / 100.0;
    }

    ImGui::EndChild();
    if(ImGui::Button("Edit")) {
      if(!self.input_name_.empty()) {
        app_state.push_action(
         [edited_profile_name = self.selected_profile_name_,
          name = self.input_name_,
          initial_capital = self.input_initial_capital_,
          capital_risk = self.input_capital_risk_](AppStateData& state) {
           auto& profiles = state.profiles;
           const auto find_it =
            std::find_if(profiles.cbegin(),
                         profiles.cend(),
                         [&edited_profile_name](const auto& profile) {
                           return profile->name() == edited_profile_name;
                         });

           if(find_it == profiles.cend()) {
             const auto error_message = std::format(
              "Profile '{}' not found for Edit.", edited_profile_name);
             state.alert_messages.push(error_message);
             return;
           }

           auto profile = *find_it;
           profile->name(name);
           profile->initial_capital(initial_capital);
           profile->capital_risk(capital_risk);

           auto& backtests = state.backtests;
           for(auto& backtest : backtests) {
             if(backtest.profile_ptr() == profile) {
               backtest.reset();
             }
           }
         });

        self.current_page_ = ProfilePage::List;
      } else {
        app_state.push_action([](AppStateData& state) {
          state.alert_messages.push("Please enter a profile name.");
        });
      }
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.current_page_ = ProfilePage::List;
    }

    ImGui::EndGroup();
  }
};

} // namespace pludux::apps
