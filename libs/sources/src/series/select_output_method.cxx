module;

#include <utility>

export module pludux:series.select_output_method;

import :asset_snapshot;
import :series.method_contextable;
import :series.method_output;

export namespace pludux::series {

template<typename TSourceMethod>
  requires requires { typename TSourceMethod::ResultType; }
class SelectOutputMethod {
public:
  using ResultType = TSourceMethod::ResultType;

  SelectOutputMethod(TSourceMethod source, MethodOutput output)
  : source_{std::move(source)}
  , output_{output}
  {
  }

  template<typename UMethod>
    requires requires { typename UMethod::ResultType; }
  SelectOutputMethod(const SelectOutputMethod<UMethod>& other,
                     MethodOutput output)
  : SelectOutputMethod{other.source(), output}
  {
  }

  auto operator==(const SelectOutputMethod& other) const noexcept
   -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_data,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    return self.source_(asset_data, self.output(), context);
  }

  auto operator()(this const auto& self,
                  AssetSnapshot asset_data,
                  MethodOutput output,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    return self.source_(asset_data, output, context);
  }

  auto source(this const auto& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this auto& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto output(this const auto& self) noexcept -> MethodOutput
  {
    return self.output_;
  }

  void output(this auto& self, MethodOutput output_name) noexcept
  {
    self.output_ = std::move(output_name);
  }

private:
  TSourceMethod source_;
  MethodOutput output_;
};

template<typename UMethod>
  requires requires { typename UMethod::ResultType; }
SelectOutputMethod(const SelectOutputMethod<UMethod>& other,
                   MethodOutput output) -> SelectOutputMethod<UMethod>;

} // namespace pludux::series