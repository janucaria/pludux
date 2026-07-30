module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <expected>
#include <format>
#include <iterator>
#include <optional>
#include <string>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>

export module pludux:ordered_named_registry;

export namespace pludux {

template<typename TValue>
class OrderedNamedRegistry {
public:
  class Iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<const std::string, TValue>;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    Iterator(std::vector<std::string>& ordered_names,
             std::unordered_map<std::string, TValue>& values)
    : ordered_names_{ordered_names}
    , values_{values}
    , index_{0}
    {
    }

    auto operator*() const -> reference
    {
      assert(index_ < ordered_names_.size());
      const auto& name = ordered_names_[index_];
      if(!values_.contains(name)) {
        assert(false);
      }
      return *values_.find(name);
    }

    auto operator++() -> Iterator&
    {
      ++index_;
      return *this;
    }

    auto operator++(int) -> Iterator
    {
      auto temp = *this;
      ++*this;
      return temp;
    }

    auto operator==(std::default_sentinel_t) const noexcept -> bool
    {
      return index_ >= ordered_names_.size();
    }

    auto operator!=(std::default_sentinel_t) const noexcept -> bool
    {
      return !(*this == std::default_sentinel);
    }

  private:
    std::vector<std::string>& ordered_names_;
    std::unordered_map<std::string, TValue>& values_;
    std::size_t index_;
  };

  class ConstIterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = const std::pair<const std::string, TValue>;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    ConstIterator(const std::vector<std::string>& ordered_names,
                  const std::unordered_map<std::string, TValue>& values)
    : ordered_names_{ordered_names}
    , values_{values}
    , index_{0}
    {
    }

    auto operator*() const -> reference
    {
      assert(index_ < ordered_names_.size());
      const auto& name = ordered_names_[index_];
      assert(values_.contains(name));
      return *values_.find(name);
    }

    auto operator++() -> ConstIterator&
    {
      ++index_;
      return *this;
    }

    auto operator++(int) -> ConstIterator
    {
      auto temp = *this;
      ++*this;
      return temp;
    }

    auto operator==(std::default_sentinel_t) const noexcept -> bool
    {
      return index_ >= ordered_names_.size();
    }

    auto operator!=(std::default_sentinel_t) const noexcept -> bool
    {
      return !(*this == std::default_sentinel);
    }

  private:
    const std::vector<std::string>& ordered_names_;
    const std::unordered_map<std::string, TValue>& values_;
    std::size_t index_;
  };

  OrderedNamedRegistry() = default;

  OrderedNamedRegistry(std::vector<std::string> ordered_names,
                       std::unordered_map<std::string, TValue> values)
  : ordered_names_{std::move(ordered_names)}
  , values_{std::move(values)}
  {
    assert(ordered_names_.size() == values_.size());
    for(const auto& name : ordered_names_) {
      assert(values_.contains(name));
    }
  }

  auto operator==(const OrderedNamedRegistry& other) const -> bool
  {
    if(ordered_names_.size() != other.ordered_names_.size()) {
      return false;
    }

    for(std::size_t i = 0; i < ordered_names_.size(); ++i) {
      if(ordered_names_[i] != other.ordered_names_[i]) {
        return false;
      }
    }

    for(const auto& name : ordered_names_) {
      const auto this_it = values_.find(name);
      const auto other_it = other.values_.find(name);

      if((this_it == values_.end()) != (other_it == other.values_.end())) {
        return false;
      }

      if(this_it != values_.end() && !(this_it->second == other_it->second)) {
        return false;
      }
    }

    return true;
  }

  auto ordered_names(this const OrderedNamedRegistry& self) noexcept
   -> const std::vector<std::string>&
  {
    return self.ordered_names_;
  }

  void ordered_names(this OrderedNamedRegistry& self,
                     std::vector<std::string> new_ordered_names) noexcept
  {
    self.ordered_names_ = std::move(new_ordered_names);
  }

  auto values(this const OrderedNamedRegistry& self) noexcept
   -> const std::unordered_map<std::string, TValue>&
  {
    return self.values_;
  }

  void values(this OrderedNamedRegistry& self,
              std::unordered_map<std::string, TValue> new_values) noexcept
  {
    self.values_ = std::move(new_values);
  }

  void set(this OrderedNamedRegistry& self,
           const std::string& name,
           TValue value) noexcept
  {
    if(self.values_.contains(name)) {
      self.values_.at(name) = std::move(value);
    } else {
      self.ordered_names_.emplace_back(name);
      self.values_.emplace(name, std::move(value));
    }
  }

  auto get(this const OrderedNamedRegistry& self, const std::string& name)
   -> std::optional<TValue>
  {
    const auto it = self.values_.find(name);
    if(it == self.values_.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  auto has(this const OrderedNamedRegistry& self, const std::string& name)
   -> bool
  {
    return self.values_.contains(name);
  }

  auto remove(this OrderedNamedRegistry& self, const std::string& name) noexcept
   -> std::optional<TValue>
  {
    const auto it = self.values_.find(name);
    if(it == self.values_.end()) {
      return std::nullopt;
    }

    auto value = it->second;
    self.values_.erase(it);

    std::erase(self.ordered_names_, name);

    return value;
  }

  auto rename(this OrderedNamedRegistry& self,
              const std::string& old_name,
              const std::string& new_name)
   -> std::expected<void, std::system_error>
  {
    if(!self.has(old_name)) {
      return std::unexpected{std::system_error{
       std::make_error_code(std::errc::invalid_argument),
       std::format("Cannot change name of non-existing name '{}'", old_name)}};
    }

    if(old_name != new_name) {
      auto order_it = std::ranges::find(self.ordered_names_, old_name);
      auto node = self.values_.extract(old_name);
      if(node.empty() || order_it == self.ordered_names_.end()) {
        return std::unexpected{std::system_error{
         std::make_error_code(std::errc::identifier_removed),
         std::format("Failed to extract value with name '{}'", old_name)}};
      }

      auto new_it = std::ranges::find(self.ordered_names_, new_name);
      if(self.values_.contains(new_name)) {
        std::erase(self.ordered_names_, new_name);
        self.values_.extract(new_name);

        order_it = std::ranges::find(self.ordered_names_, old_name);
      }

      node.key() = new_name;
      self.values_.insert(std::move(node));
      *order_it = new_name;

      assert(self.values_.size() == self.ordered_names_.size());
    }

    return std::expected<void, std::system_error>{};
  }

  auto begin() noexcept -> Iterator
  {
    return Iterator{ordered_names_, values_};
  }

  auto begin() const noexcept -> ConstIterator
  {
    return ConstIterator{ordered_names_, values_};
  }

  auto end() const noexcept -> std::default_sentinel_t
  {
    return std::default_sentinel;
  }

  auto size(this const OrderedNamedRegistry& self) noexcept -> size_t
  {
    assert(self.values_.size() == self.ordered_names_.size());
    return self.values_.size();
  }

  auto empty(this const OrderedNamedRegistry& self) noexcept -> bool
  {
    assert(self.values_.size() == self.ordered_names_.size());
    return self.values_.empty();
  }

private:
  std::vector<std::string> ordered_names_;
  std::unordered_map<std::string, TValue> values_;
};

} // namespace pludux