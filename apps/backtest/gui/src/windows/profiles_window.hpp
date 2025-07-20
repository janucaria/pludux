#ifndef PLUDUX_APPS_BACKTEST_PROFILES_WINDOW_HPP
#define PLUDUX_APPS_BACKTEST_PROFILES_WINDOW_HPP

#include "../app_state.hpp"

namespace pludux::apps {

class ProfilesWindow {
public:
  ProfilesWindow();

  void render(AppState& app_state);

private:
  enum class ProfilePage {
    List,
    AddNewProfile,
    EditProfile
  } current_page_ = ProfilePage::List;

  std::string selected_profile_name_;
  
  std::string input_name_;
  double input_initial_capital_ = 100'000'000.0;
  double input_capital_risk_ = 0.01;

  void render_profiles_list(AppState& app_state);

  void render_add_new_profile(AppState& app_state);

  void render_edit_profile(AppState& app_state);
};

} // namespace pludux::apps

#endif
