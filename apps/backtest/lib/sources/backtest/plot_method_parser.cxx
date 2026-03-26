module;

#include <cstdint>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <ctre.hpp>
#include <jsoncons/json.hpp>

export module pludux.backtest:plot_method_parser;

import :plots;
import :method_serializer_mixin;
import :plot_source_method_parser;

export namespace pludux::backtest {

class PlotMethodParser
: public MethodSerializerMixin<PlotMethodParser, AnyPlotMethod> {
public:
  using MethodSerializerMixin<PlotMethodParser,
                              AnyPlotMethod>::MethodSerializerMixin;

  PlotMethodParser(PlotSourceMethodParser plot_source_method_parser)
  : MethodSerializerMixin<PlotMethodParser, AnyPlotMethod>{}
  , plot_source_method_parser_{std::move(plot_source_method_parser)}
  {
  }

  auto plot_source_method_parser(this const PlotMethodParser& self) noexcept
   -> const PlotSourceMethodParser&
  {
    return self.plot_source_method_parser_;
  }

  void plot_source_method_parser(
   this PlotMethodParser& self,
   PlotSourceMethodParser new_plot_source_method_parser)
  {
    self.plot_source_method_parser_ = std::move(new_plot_source_method_parser);
  }

  auto serialize_plot_source_method(this const PlotMethodParser& self,
                                    const AnyPlotSourceMethod& method)
   -> jsoncons::ojson
  {
    return self.plot_source_method_parser_.serialize_method(method);
  }

private:
  PlotSourceMethodParser plot_source_method_parser_;
};

auto make_default_registered_plot_method_parser() -> PlotMethodParser;

} // namespace pludux::backtest

namespace pludux::backtest {

auto string_color_to_u32(std::string color) -> std::uint32_t
{
  auto pack_rgba = [](int r, int g, int b, int a) -> std::uint32_t {
    const auto clamp_channel = [](int value) {
      return std::clamp(value, 0, 255);
    };

    const auto rr = static_cast<std::uint32_t>(clamp_channel(r));
    const auto gg = static_cast<std::uint32_t>(clamp_channel(g));
    const auto bb = static_cast<std::uint32_t>(clamp_channel(b));
    const auto aa = static_cast<std::uint32_t>(clamp_channel(a));

    // Keep RGBA byte order consistent with previous IM_COL32 usage.
    return rr | (gg << 8) | (bb << 16) | (aa << 24);
  };

  auto float01_to_byte = [](float value) {
    const auto v = std::clamp(value, 0.0f, 1.0f);
    return static_cast<int>(v * 255.0f + 0.5f);
  };

  auto trim = [](std::string& s) {
    const auto start = s.find_first_not_of(" \t\n\r");
    if(start == std::string::npos) {
      s.clear();
      return;
    }

    const auto end = s.find_last_not_of(" \t\n\r");
    s = s.substr(start, end - start + 1);
  };

  auto to_lowercase = [](std::string& s) {
    std::ranges::transform(s, s.begin(), [](unsigned char ch) {
      return static_cast<char>(std::tolower(ch));
    });
  };

  auto clamp01 = [](float value) { return std::clamp(value, 0.0f, 1.0f); };

  auto hex_digit = [](char ch) -> int {
    if(ch >= '0' && ch <= '9') {
      return ch - '0';
    }
    if(ch >= 'a' && ch <= 'f') {
      return 10 + (ch - 'a');
    }
    if(ch >= 'A' && ch <= 'F') {
      return 10 + (ch - 'A');
    }
    return -1;
  };

  auto parse_hex_byte = [&](char hi, char lo) -> int {
    const auto high = hex_digit(hi);
    const auto low = hex_digit(lo);
    if(high < 0 || low < 0) {
      return -1;
    }
    return (high << 4) | low;
  };

  trim(color);
  to_lowercase(color);

  static const auto named_colors =
   std::unordered_map<std::string_view, std::uint32_t>{
    {"aliceblue", pack_rgba(240, 248, 255, 255)},
    {"antiquewhite", pack_rgba(250, 235, 215, 255)},
    {"aqua", pack_rgba(0, 255, 255, 255)},
    {"aquamarine", pack_rgba(127, 255, 212, 255)},
    {"azure", pack_rgba(240, 255, 255, 255)},
    {"beige", pack_rgba(245, 245, 220, 255)},
    {"bisque", pack_rgba(255, 228, 196, 255)},
    {"black", pack_rgba(0, 0, 0, 255)},
    {"blanchedalmond", pack_rgba(255, 235, 205, 255)},
    {"blue", pack_rgba(0, 0, 255, 255)},
    {"blueviolet", pack_rgba(138, 43, 226, 255)},
    {"brown", pack_rgba(165, 42, 42, 255)},
    {"burlywood", pack_rgba(222, 184, 135, 255)},
    {"cadetblue", pack_rgba(95, 158, 160, 255)},
    {"chartreuse", pack_rgba(127, 255, 0, 255)},
    {"chocolate", pack_rgba(210, 105, 30, 255)},
    {"coral", pack_rgba(255, 127, 80, 255)},
    {"cornflowerblue", pack_rgba(100, 149, 237, 255)},
    {"cornsilk", pack_rgba(255, 248, 220, 255)},
    {"crimson", pack_rgba(220, 20, 60, 255)},
    {"cyan", pack_rgba(0, 255, 255, 255)},
    {"darkblue", pack_rgba(0, 0, 139, 255)},
    {"darkcyan", pack_rgba(0, 139, 139, 255)},
    {"darkgoldenrod", pack_rgba(184, 134, 11, 255)},
    {"darkgray", pack_rgba(169, 169, 169, 255)},
    {"darkgreen", pack_rgba(0, 100, 0, 255)},
    {"darkgrey", pack_rgba(169, 169, 169, 255)},
    {"darkkhaki", pack_rgba(189, 183, 107, 255)},
    {"darkmagenta", pack_rgba(139, 0, 139, 255)},
    {"darkolivegreen", pack_rgba(85, 107, 47, 255)},
    {"darkorange", pack_rgba(255, 140, 0, 255)},
    {"darkorchid", pack_rgba(153, 50, 204, 255)},
    {"darkred", pack_rgba(139, 0, 0, 255)},
    {"darksalmon", pack_rgba(233, 150, 122, 255)},
    {"darkseagreen", pack_rgba(143, 188, 143, 255)},
    {"darkslateblue", pack_rgba(72, 61, 139, 255)},
    {"darkslategray", pack_rgba(47, 79, 79, 255)},
    {"darkslategrey", pack_rgba(47, 79, 79, 255)},
    {"darkturquoise", pack_rgba(0, 206, 209, 255)},
    {"darkviolet", pack_rgba(148, 0, 211, 255)},
    {"deeppink", pack_rgba(255, 20, 147, 255)},
    {"deepskyblue", pack_rgba(0, 191, 255, 255)},
    {"dimgray", pack_rgba(105, 105, 105, 255)},
    {"dimgrey", pack_rgba(105, 105, 105, 255)},
    {"dodgerblue", pack_rgba(30, 144, 255, 255)},
    {"firebrick", pack_rgba(178, 34, 34, 255)},
    {"floralwhite", pack_rgba(255, 250, 240, 255)},
    {"forestgreen", pack_rgba(34, 139, 34, 255)},
    {"fuchsia", pack_rgba(255, 0, 255, 255)},
    {"gainsboro", pack_rgba(220, 220, 220, 255)},
    {"ghostwhite", pack_rgba(248, 248, 255, 255)},
    {"gold", pack_rgba(255, 215, 0, 255)},
    {"goldenrod", pack_rgba(218, 165, 32, 255)},
    {"gray", pack_rgba(128, 128, 128, 255)},
    {"green", pack_rgba(0, 128, 0, 255)},
    {"greenyellow", pack_rgba(173, 255, 47, 255)},
    {"grey", pack_rgba(128, 128, 128, 255)},
    {"honeydew", pack_rgba(240, 255, 240, 255)},
    {"hotpink", pack_rgba(255, 105, 180, 255)},
    {"indianred", pack_rgba(205, 92, 92, 255)},
    {"indigo", pack_rgba(75, 0, 130, 255)},
    {"ivory", pack_rgba(255, 255, 240, 255)},
    {"khaki", pack_rgba(240, 230, 140, 255)},
    {"lavender", pack_rgba(230, 230, 250, 255)},
    {"lavenderblush", pack_rgba(255, 240, 245, 255)},
    {"lawngreen", pack_rgba(124, 252, 0, 255)},
    {"lemonchiffon", pack_rgba(255, 250, 205, 255)},
    {"lightblue", pack_rgba(173, 216, 230, 255)},
    {"lightcoral", pack_rgba(240, 128, 128, 255)},
    {"lightcyan", pack_rgba(224, 255, 255, 255)},
    {"lightgoldenrodyellow", pack_rgba(250, 250, 210, 255)},
    {"lightgray", pack_rgba(211, 211, 211, 255)},
    {"lightgreen", pack_rgba(144, 238, 144, 255)},
    {"lightgrey", pack_rgba(211, 211, 211, 255)},
    {"lightpink", pack_rgba(255, 182, 193, 255)},
    {"lightsalmon", pack_rgba(255, 160, 122, 255)},
    {"lightseagreen", pack_rgba(32, 178, 170, 255)},
    {"lightskyblue", pack_rgba(135, 206, 250, 255)},
    {"lightslategray", pack_rgba(119, 136, 153, 255)},
    {"lightslategrey", pack_rgba(119, 136, 153, 255)},
    {"lightsteelblue", pack_rgba(176, 196, 222, 255)},
    {"lightyellow", pack_rgba(255, 255, 224, 255)},
    {"lime", pack_rgba(0, 255, 0, 255)},
    {"limegreen", pack_rgba(50, 205, 50, 255)},
    {"linen", pack_rgba(250, 240, 230, 255)},
    {"magenta", pack_rgba(255, 0, 255, 255)},
    {"maroon", pack_rgba(128, 0, 0, 255)},
    {"mediumaquamarine", pack_rgba(102, 205, 170, 255)},
    {"mediumblue", pack_rgba(0, 0, 205, 255)},
    {"mediumorchid", pack_rgba(186, 85, 211, 255)},
    {"mediumpurple", pack_rgba(147, 112, 219, 255)},
    {"mediumseagreen", pack_rgba(60, 179, 113, 255)},
    {"mediumslateblue", pack_rgba(123, 104, 238, 255)},
    {"mediumspringgreen", pack_rgba(0, 250, 154, 255)},
    {"mediumturquoise", pack_rgba(72, 209, 204, 255)},
    {"mediumvioletred", pack_rgba(199, 21, 133, 255)},
    {"midnightblue", pack_rgba(25, 25, 112, 255)},
    {"mintcream", pack_rgba(245, 255, 250, 255)},
    {"mistyrose", pack_rgba(255, 228, 225, 255)},
    {"moccasin", pack_rgba(255, 228, 181, 255)},
    {"navajowhite", pack_rgba(255, 222, 173, 255)},
    {"navy", pack_rgba(0, 0, 128, 255)},
    {"oldlace", pack_rgba(253, 245, 230, 255)},
    {"olive", pack_rgba(128, 128, 0, 255)},
    {"olivedrab", pack_rgba(107, 142, 35, 255)},
    {"orange", pack_rgba(255, 165, 0, 255)},
    {"orangered", pack_rgba(255, 69, 0, 255)},
    {"orchid", pack_rgba(218, 112, 214, 255)},
    {"palegoldenrod", pack_rgba(238, 232, 170, 255)},
    {"palegreen", pack_rgba(152, 251, 152, 255)},
    {"paleturquoise", pack_rgba(175, 238, 238, 255)},
    {"palevioletred", pack_rgba(219, 112, 147, 255)},
    {"papayawhip", pack_rgba(255, 239, 213, 255)},
    {"peachpuff", pack_rgba(255, 218, 185, 255)},
    {"peru", pack_rgba(205, 133, 63, 255)},
    {"pink", pack_rgba(255, 192, 203, 255)},
    {"plum", pack_rgba(221, 160, 221, 255)},
    {"powderblue", pack_rgba(176, 224, 230, 255)},
    {"purple", pack_rgba(128, 0, 128, 255)},
    {"rebeccapurple", pack_rgba(102, 51, 153, 255)},
    {"red", pack_rgba(255, 0, 0, 255)},
    {"rosybrown", pack_rgba(188, 143, 143, 255)},
    {"royalblue", pack_rgba(65, 105, 225, 255)},
    {"saddlebrown", pack_rgba(139, 69, 19, 255)},
    {"salmon", pack_rgba(250, 128, 114, 255)},
    {"sandybrown", pack_rgba(244, 164, 96, 255)},
    {"seagreen", pack_rgba(46, 139, 87, 255)},
    {"seashell", pack_rgba(255, 245, 238, 255)},
    {"sienna", pack_rgba(160, 82, 45, 255)},
    {"silver", pack_rgba(192, 192, 192, 255)},
    {"skyblue", pack_rgba(135, 206, 235, 255)},
    {"slateblue", pack_rgba(106, 90, 205, 255)},
    {"slategray", pack_rgba(112, 128, 144, 255)},
    {"slategrey", pack_rgba(112, 128, 144, 255)},
    {"snow", pack_rgba(255, 250, 250, 255)},
    {"springgreen", pack_rgba(0, 255, 127, 255)},
    {"steelblue", pack_rgba(70, 130, 180, 255)},
    {"tan", pack_rgba(210, 180, 140, 255)},
    {"teal", pack_rgba(0, 128, 128, 255)},
    {"thistle", pack_rgba(216, 191, 216, 255)},
    {"tomato", pack_rgba(255, 99, 71, 255)},
    {"turquoise", pack_rgba(64, 224, 208, 255)},
    {"violet", pack_rgba(238, 130, 238, 255)},
    {"wheat", pack_rgba(245, 222, 179, 255)},
    {"white", pack_rgba(255, 255, 255, 255)},
    {"whitesmoke", pack_rgba(245, 245, 245, 255)},
    {"yellow", pack_rgba(255, 255, 0, 255)},
    {"yellowgreen", pack_rgba(154, 205, 50, 255)}};

  if(const auto it = named_colors.find(color); it != named_colors.end()) {
    return it->second;
  }

  if(!color.empty() && color[0] == '#') {
    const auto len = color.size();
    auto r = 255;
    auto g = 255;
    auto b = 255;
    auto a = 255;

    if(len == 4 || len == 5) {
      const auto r1 = hex_digit(color[1]);
      const auto g1 = hex_digit(color[2]);
      const auto b1 = hex_digit(color[3]);
      if(r1 >= 0 && g1 >= 0 && b1 >= 0) {
        r = (r1 << 4) | r1;
        g = (g1 << 4) | g1;
        b = (b1 << 4) | b1;

        if(len == 5) {
          const auto a1 = hex_digit(color[4]);
          if(a1 >= 0) {
            a = (a1 << 4) | a1;
          }
        }

        return pack_rgba(r, g, b, a);
      }
    }

    if(len == 7 || len == 9) {
      r = parse_hex_byte(color[1], color[2]);
      g = parse_hex_byte(color[3], color[4]);
      b = parse_hex_byte(color[5], color[6]);
      if(r >= 0 && g >= 0 && b >= 0) {
        if(len == 9) {
          const auto parsed_alpha = parse_hex_byte(color[7], color[8]);
          if(parsed_alpha >= 0) {
            a = parsed_alpha;
          }
        }

        return pack_rgba(r, g, b, a);
      }
    }
  }

  if(
   const auto match = ctre::match<
    R"(^rgba?\(\s*(\d{1,3})\s*,\s*(\d{1,3})\s*,\s*(\d{1,3})(?:\s*,\s*([0-9]*\.?[0-9]+))?\s*\)$)">(
    color)) {
    const auto r = std::stof(std::string{match.get<1>().to_view()}) / 255.0f;
    const auto g = std::stof(std::string{match.get<2>().to_view()}) / 255.0f;
    const auto b = std::stof(std::string{match.get<3>().to_view()}) / 255.0f;
    const auto alpha_sv = match.get<4>().to_view();

    auto a = 1.0f;
    if(!alpha_sv.empty()) {
      const auto parsed_alpha = std::stof(std::string{alpha_sv});
      a = parsed_alpha > 1.0f ? parsed_alpha / 255.0f : parsed_alpha;
    }

    return pack_rgba(float01_to_byte(clamp01(r)),
                     float01_to_byte(clamp01(g)),
                     float01_to_byte(clamp01(b)),
                     float01_to_byte(clamp01(a)));
  }

  return pack_rgba(255, 255, 255, 255);
}

auto make_default_registered_plot_method_parser() -> PlotMethodParser
{
  auto method_parser =
   PlotMethodParser{make_default_registered_plot_source_method_parser()};

  method_parser.register_method_parser(
   "HLINE",
   [](const PlotMethodParser& method_parser,
      const AnyPlotMethod any_plot_method) -> jsoncons::ojson {
     auto serialized_method = jsoncons::ojson::null();

     auto plot_method = plot_method_cast<HLinePlotMethod>(any_plot_method);
     if(plot_method) {
       serialized_method = jsoncons::ojson{};
       serialized_method["level"] = plot_method->level();
       serialized_method["color"] = plot_method->color();
     }

     return serialized_method;
   },
   [](const PlotMethodParser& method_parser,
      const jsoncons::ojson& parameters) {
     const auto level =
      method_parser.get_param_or<double>(parameters, "level", 0.0);

     auto color = std::uint32_t{0xFFFFFFFF};
     if(parameters.contains("color")) {
       const auto color_param = parameters.at("color");
       if(color_param.is_string()) {
         color = string_color_to_u32(color_param.as_string());
       } else if(color_param.is_number()) {
         color = color_param.as<std::uint32_t>();
       }
     }

     return HLinePlotMethod{level, color};
   });

  method_parser.register_method_parser(
   "LINE",
   [](const PlotMethodParser& method_parser,
      const AnyPlotMethod any_plot_method) -> jsoncons::ojson {
     auto serialized_method = jsoncons::ojson::null();

     auto plot_method =
      plot_method_cast<LinePlotMethod<AnyPlotSourceMethod>>(any_plot_method);
     if(plot_method) {
       serialized_method = jsoncons::ojson{};
       serialized_method["source"] =
        method_parser.serialize_plot_source_method(plot_method->source());
       serialized_method["color"] = plot_method->color();
     }

     return serialized_method;
   },
   [](const PlotMethodParser& method_parser,
      const jsoncons::ojson& parameters) {
     const auto source =
      method_parser.plot_source_method_parser().parse_method_from_param_or(
       method_parser.plot_source_method_parser(),
       parameters,
       "source",
       SeriesPlotSourceMethod{});

     auto color = std::uint32_t{0xFFFFFFFF};
     if(parameters.contains("color")) {
       const auto color_param = parameters.at("color");
       if(color_param.is_string()) {
         color = string_color_to_u32(color_param.as_string());
       } else if(color_param.is_number()) {
         color = color_param.as<std::uint32_t>();
       }
     }

     return LinePlotMethod<AnyPlotSourceMethod>{source, color};
   });

  method_parser.register_method_parser(
   "HISTOGRAM",
   [](const PlotMethodParser& method_parser,
      const AnyPlotMethod any_plot_method) -> jsoncons::ojson {
     auto serialized_method = jsoncons::ojson::null();

     auto plot_method =
      plot_method_cast<HistogramPlotMethod<AnyPlotSourceMethod>>(any_plot_method);
     if(plot_method) {
       serialized_method = jsoncons::ojson{};
       serialized_method["source"] =
        method_parser.serialize_plot_source_method(plot_method->source());
       serialized_method["color"] = plot_method->color();
     }

     return serialized_method;
   },
   [](const PlotMethodParser& method_parser,
      const jsoncons::ojson& parameters) {
     const auto source =
      method_parser.plot_source_method_parser().parse_method_from_param_or(
       method_parser.plot_source_method_parser(),
       parameters,
       "source",
       SeriesPlotSourceMethod{});

     auto color = std::uint32_t{0xFFFFFFFF};
     if(parameters.contains("color")) {
       const auto color_param = parameters.at("color");
       if(color_param.is_string()) {
         color = string_color_to_u32(color_param.as_string());
       } else if(color_param.is_number()) {
         color = color_param.as<std::uint32_t>();
       }
     }

     return HistogramPlotMethod<AnyPlotSourceMethod>{source, color};
   });

  return method_parser;
}

} // namespace pludux::backtest
