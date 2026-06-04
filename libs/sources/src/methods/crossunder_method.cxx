module;

#include <utility>

export module pludux:methods.crossunder_method;

export namespace pludux {

template<typename TSourceMethod, typename TReferenceMethod>
class CrossunderMethod {
public:
  CrossunderMethod(TSourceMethod source, TReferenceMethod reference)
  : source_{std::move(source)}
  , reference_{std::move(reference)}
  {
  }

  auto operator==(const CrossunderMethod& other) const noexcept
   -> bool = default;

  auto source(this const CrossunderMethod& self) noexcept
   -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this CrossunderMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto reference(this const CrossunderMethod& self) noexcept
   -> const TReferenceMethod&
  {
    return self.reference_;
  }

  void reference(this CrossunderMethod& self,
                 TReferenceMethod reference) noexcept
  {
    self.reference_ = std::move(reference);
  }

private:
  TSourceMethod source_;
  TReferenceMethod reference_;
};

} // namespace pludux
