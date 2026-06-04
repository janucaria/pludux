module;

#include <utility>

export module pludux:methods.select_output_method;

export namespace pludux {

enum class MethodOutput {
  UpperBand,
  MiddleBand,
  LowerBand,
  MacdLine,
  SignalLine,
  Histogram,
  KPercent,
  DPercent
};

template<typename TSourceMethod>
class SelectOutputMethod {
public:
  SelectOutputMethod(TSourceMethod source, MethodOutput output)
  : source_{std::move(source)}
  , output_{output}
  {
  }

  template<typename UMethod>
  SelectOutputMethod(const SelectOutputMethod<UMethod>& other,
                     MethodOutput output)
  : SelectOutputMethod{other.source(), output}
  {
  }

  auto operator==(const SelectOutputMethod& other) const noexcept
   -> bool = default;

  auto source(this const SelectOutputMethod& self) noexcept
   -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this SelectOutputMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto output(this const SelectOutputMethod& self) noexcept -> MethodOutput
  {
    return self.output_;
  }

  void output(this SelectOutputMethod& self, MethodOutput output_name) noexcept
  {
    self.output_ = std::move(output_name);
  }

private:
  TSourceMethod source_;
  MethodOutput output_;
};

// Deduction guide: SelectOutputMethod{other_select, output} deduces TSourceMethod
// from the inner source of other_select, not from SelectOutputMethod itself.
template<typename UMethod>
SelectOutputMethod(const SelectOutputMethod<UMethod>&, MethodOutput)
 -> SelectOutputMethod<UMethod>;

} // namespace pludux