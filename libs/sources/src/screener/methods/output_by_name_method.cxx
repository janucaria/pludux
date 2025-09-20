module;

#include <utility>

export module pludux:screener.output_by_name_method;

import :asset_snapshot;
import :screener.screener_method;

export namespace pludux::screener {

class OutputByNameMethod {
public:
  OutputByNameMethod(ScreenerMethod source, OutputName output_name)
  : source_{std::move(source)}
  , output_name_{output_name}
  {
  }

  auto operator==(const OutputByNameMethod& other) const noexcept
   -> bool = default;

  auto operator()(this const auto& self, AssetSnapshot asset_data)
   -> PolySeries<double>
  {
    return OutputByNameSeries{self.source_(asset_data), self.output_name_};
  }

  auto source(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.source_;
  }

  void source(this auto& self, ScreenerMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto output(this const auto& self) noexcept -> OutputName
  {
    return self.output_name_;
  }

  void output(this auto& self, OutputName output_name) noexcept
  {
    self.output_name_ = std::move(output_name);
  }

private:
  ScreenerMethod source_;
  OutputName output_name_;
};

} // namespace pludux::screener