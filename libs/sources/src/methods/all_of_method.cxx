module;

#include <initializer_list>
#include <utility>
#include <vector>

export module pludux:methods.all_of_method;

export namespace pludux {

template<typename TMethod>
class AllOfMethod {
public:
  AllOfMethod() = default;

  AllOfMethod(std::initializer_list<TMethod> conditions)
  : conditions_{conditions}
  {
  }

  explicit AllOfMethod(std::vector<TMethod> conditions)
  : conditions_{std::move(conditions)}
  {
  }

  auto operator==(const AllOfMethod& other) const noexcept -> bool = default;

  auto conditions(this const AllOfMethod& self) noexcept
   -> const std::vector<TMethod>&
  {
    return self.conditions_;
  }

  void conditions(this AllOfMethod& self,
                  std::vector<TMethod> conditions) noexcept
  {
    self.conditions_ = std::move(conditions);
  }

private:
  std::vector<TMethod> conditions_;
};

} // namespace pludux