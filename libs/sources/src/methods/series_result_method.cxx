module;

#include <string>
#include <utility>

export module pludux:methods.series_result_method;

export namespace pludux {

class SeriesResultMethod {
public:
  SeriesResultMethod(std::string name)
  : name_{std::move(name)}
  {
  }

  auto operator==(const SeriesResultMethod& other) const noexcept -> bool
  {
    return name_ == other.name_;
  }

  auto operator!=(const SeriesResultMethod& other) const noexcept -> bool
  {
    return !(*this == other);
  }

  auto name(this const SeriesResultMethod& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this SeriesResultMethod& self, std::string new_name) noexcept
  {
    self.name_ = std::move(new_name);
  }

private:
  std::string name_{};
};

} // namespace pludux
