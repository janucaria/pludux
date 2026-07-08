module;

#include <string>
#include <utility>

export module pludux:methods.series_method;

export namespace pludux {

class SeriesMethod {
public:
  SeriesMethod(std::string name)
  : name_{std::move(name)}
  {
  }

  auto operator==(const SeriesMethod& other) const noexcept -> bool
  {
    return name_ == other.name_;
  }

  auto operator!=(const SeriesMethod& other) const noexcept -> bool
  {
    return !(*this == other);
  }

  auto name(this const SeriesMethod& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this SeriesMethod& self, std::string new_name) noexcept
  {
    self.name_ = std::move(new_name);
  }

private:
  std::string name_{};
};

} // namespace pludux
