module;

#include <imgui.h>
#include <implot.h>

export module pludux.apps.backtest:ui.theme;

export namespace pludux::apps::ui {

void apply_dark_theme()
{
  ImGui::StyleColorsDark();
  ImPlot::StyleColorsDark();

  auto& style = ImGui::GetStyle();
  style.WindowRounding = 6.0f;
  style.ChildRounding = 6.0f;
  style.FrameRounding = 5.0f;
  style.PopupRounding = 6.0f;
  style.ScrollbarRounding = 8.0f;
  style.GrabRounding = 5.0f;
  style.FramePadding = ImVec2{8.0f, 5.0f};
  style.ItemSpacing = ImVec2{8.0f, 6.0f};
  style.TouchExtraPadding = ImVec2{};
  style.FontScaleMain = 1.0f;
}

} // namespace pludux::apps::ui
