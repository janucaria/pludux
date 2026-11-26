#pragma once

#include <algorithm>

#include <imgui.h>

namespace pludux::apps {

inline void fill_chart_bar(ImDrawList& draw_list,
                           const ImVec2& left_corner,
                           const ImVec2& right_corner,
                           ImU32 color)
{
  const auto top = std::min(left_corner.y, right_corner.y);
  const auto bottom = std::max(left_corner.y, right_corner.y);
  draw_list.AddRectFilled(
   {left_corner.x, top}, {right_corner.x, bottom}, color);
}

} // namespace pludux::apps
