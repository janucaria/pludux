module;

#include <functional>
#include <stdexcept>
#include <string>
#include <utility>

#include <jsoncons/json.hpp>

export module pludux.backtest:plot_source_method_parser;

import :plots;
import :method_serializer_mixin;

export namespace pludux::backtest {

class PlotSourceMethodParser
: public MethodSerializerMixin<PlotSourceMethodParser, AnyPlotSourceMethod> {
public:
  using MethodSerializerMixin<PlotSourceMethodParser,
                              AnyPlotSourceMethod>::MethodSerializerMixin;
};

auto make_default_registered_plot_source_method_parser()
 -> PlotSourceMethodParser;

} // namespace pludux::backtest

namespace pludux::backtest {

auto make_default_registered_plot_source_method_parser()
 -> PlotSourceMethodParser
{
  auto method_parser = PlotSourceMethodParser{};

  method_parser.register_method_parser(
   "SERIES",
   [](const PlotSourceMethodParser& method_parser,
      const AnyPlotSourceMethod any_plot_source_method) -> jsoncons::ojson {
     auto serialized_method = jsoncons::ojson::null();

     auto plot_source_method =
      plot_source_method_cast<SeriesPlotSourceMethod>(any_plot_source_method);
     if(plot_source_method) {
       serialized_method = jsoncons::ojson{};
       serialized_method["name"] = plot_source_method->series_name();
     }

     return serialized_method;
   },
   [](const PlotSourceMethodParser& method_parser,
      const jsoncons::ojson& parameters) {
     const auto series_name =
      method_parser.get_param_or(parameters, "name", std::string{});

     return SeriesPlotSourceMethod{series_name};
   });

  method_parser.register_method_parser(
   "CONSTANT",
   [](const PlotSourceMethodParser& method_parser,
      const AnyPlotSourceMethod any_plot_source_method) -> jsoncons::ojson {
     auto serialized_method = jsoncons::ojson::null();

     auto plot_source_method =
      plot_source_method_cast<ConstantPlotSourceMethod>(any_plot_source_method);
     if(plot_source_method) {
       serialized_method = jsoncons::ojson{};
       serialized_method["value"] = plot_source_method->value();
     }

     return serialized_method;
   },
   [](const PlotSourceMethodParser& method_parser,
      const jsoncons::ojson& parameters) {
     const auto value = method_parser.get_param_or(parameters, "value", 0.0);

     return ConstantPlotSourceMethod{value};
   });

  return method_parser;
}

} // namespace pludux::backtest
