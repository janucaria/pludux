module;

#include <initializer_list>
#include <utility>
#include <vector>

export module pludux:methods.any_of_method;

export namespace pludux {

template<typename TMethod>
class AnyOfMethod {
public:
  AnyOfMethod(std::initializer_list<TMethod> conditions)
  : conditions_{conditions}
  {
  }

  explicit AnyOfMethod(std::vector<TMethod> conditions)
  : conditions_{std::move(conditions)}
  {
  }

  auto operator==(const AnyOfMethod& other) const noexcept -> bool = default;

  auto conditions(this const AnyOfMethod& self) noexcept
   -> const std::vector<TMethod>&
  {
    return self.conditions_;
  }

  void conditions(this AnyOfMethod& self,
                  std::vector<TMethod> conditions) noexcept
  {
    self.conditions_ = std::move(conditions);
  }

private:
  std::vector<TMethod> conditions_;
};

} // namespace pludux