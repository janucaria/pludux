module;

#include <utility>

export module pludux:series.select_output_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

export namespace pludux {

template<typename TSourceMethod>
  requires requires { typename TSourceMethod::ResultType; }
class SelectOutputMethod {
public:
  using ResultType = TSourceMethod::ResultType;

  SelectOutputMethod(TSourceMethod source, SeriesOutput output)
  : source_{std::move(source)}
  , output_{output}
  {
  }

  template<typename UMethod>
    requires requires { typename UMethod::ResultType; }
  SelectOutputMethod(const SelectOutputMethod<UMethod>& other,
                     SeriesOutput output)
  : SelectOutputMethod{other.source(), output}
  {
  }

  auto operator==(const SelectOutputMethod& other) const noexcept
   -> bool = default;

  auto operator()(this const SelectOutputMethod& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return self.source_(asset_snapshot, self.output(), context);
  }

  auto operator()(this const SelectOutputMethod& self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return self.source_(asset_snapshot, output, context);
  }

  auto source(this const SelectOutputMethod& self) noexcept
   -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this SelectOutputMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto output(this const SelectOutputMethod& self) noexcept -> SeriesOutput
  {
    return self.output_;
  }

  void output(this SelectOutputMethod& self, SeriesOutput output_name) noexcept
  {
    self.output_ = std::move(output_name);
  }

private:
  TSourceMethod source_;
  SeriesOutput output_;
};

template<typename UMethod>
  requires requires { typename UMethod::ResultType; }
SelectOutputMethod(const SelectOutputMethod<UMethod>& other,
                   SeriesOutput output) -> SelectOutputMethod<UMethod>;

} // namespace pludux