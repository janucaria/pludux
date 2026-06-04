module;

#include <utility>

export module pludux:methods.crossover_method;

export namespace pludux {

template<typename TSourceMethod, typename TReferenceMethod>
class CrossoverMethod {
public:
  CrossoverMethod(TSourceMethod source, TReferenceMethod reference)
  : source_{std::move(source)}
  , reference_{std::move(reference)}
  {
  }

  auto operator==(const CrossoverMethod& other) const noexcept
   -> bool = default;

  auto source(this const CrossoverMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this CrossoverMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto reference(this const CrossoverMethod& self) noexcept
   -> const TReferenceMethod&
  {
    return self.reference_;
  }

  void reference(this CrossoverMethod& self,
                 TReferenceMethod reference) noexcept
  {
    self.reference_ = std::move(reference);
  }

private:
  TSourceMethod source_;
  TReferenceMethod reference_;
};

} // namespace pludux
