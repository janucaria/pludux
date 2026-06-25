module;

#include <string>
#include <utility>

export module pludux:nodes.series_node;


export namespace pludux {

class SeriesNode {
public:
  SeriesNode(std::string name)
  : name_{std::move(name)}
  {
  }

  auto operator==(const SeriesNode& other) const noexcept
   -> bool = default;


  auto name(this const SeriesNode& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this SeriesNode& self, std::string new_name) noexcept
  {
    self.name_ = std::move(new_name);
  }

private:
  std::string name_{};
};

} // namespace pludux