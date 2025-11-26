module;

#include <algorithm>
#include <cassert>
#include <expected>
#include <format>
#include <iterator>
#include <optional>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>

export module pludux:series.series_method_registry;

import :series.any_series_method;

export namespace pludux {

class SeriesMethodRegistry {
public:
  class Iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<const std::string, AnySeriesMethod>;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    Iterator(std::vector<std::string>& ordered_names,
             std::unordered_map<std::string, AnySeriesMethod>& methods)
    : ordered_names_{ordered_names}
    , methods_{methods}
    , index_{0}
    {
    }

    auto operator*() const -> reference
    {
      assert(index_ < ordered_names_.size());
      const auto& name = ordered_names_[index_];
      assert(methods_.contains(name));
      return *methods_.find(name);
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
    std::unordered_map<std::string, AnySeriesMethod>& methods_;
    std::size_t index_;
  };

  class ConstIterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = const std::pair<const std::string, AnySeriesMethod>;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    ConstIterator(
     const std::vector<std::string>& ordered_names,
     const std::unordered_map<std::string, AnySeriesMethod>& methods)
    : ordered_names_{ordered_names}
    , methods_{methods}
    , index_{0}
    {
    }

    auto operator*() const -> reference
    {
      assert(index_ < ordered_names_.size());
      const auto& name = ordered_names_[index_];
      assert(methods_.contains(name));
      return *methods_.find(name);
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
    const std::unordered_map<std::string, AnySeriesMethod>& methods_;
    std::size_t index_;
  };

  SeriesMethodRegistry() = default;

  SeriesMethodRegistry(std::unordered_map<std::string, AnySeriesMethod> methods)
  : methods_{std::move(methods)}
  {
  }

  auto methods(this const auto& self) noexcept
   -> const std::unordered_map<std::string, AnySeriesMethod>&
  {
    return self.methods_;
  }

  void
  methods(this auto& self,
          std::unordered_map<std::string, AnySeriesMethod> new_methods) noexcept
  {
    self.methods_ = std::move(new_methods);
  }

  void set(this auto& self, const std::string& name, AnySeriesMethod method)
  {
    if(self.methods_.contains(name)) {
      self.methods_.at(name) = std::move(method);
    } else {
      self.ordered_names_.emplace_back(name);
      self.methods_.emplace(name, std::move(method));
    }
  }

  auto get(this const auto& self, const std::string& name)
   -> std::optional<AnySeriesMethod>
  {
    const auto it = self.methods_.find(name);
    if(it == self.methods_.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  auto has(this const auto& self, const std::string& name) -> bool
  {
    return self.methods_.contains(name);
  }

  auto remove(this auto& self, const std::string& name) noexcept
   -> std::optional<AnySeriesMethod>
  {
    const auto it = self.methods_.find(name);
    if(it == self.methods_.end()) {
      return std::nullopt;
    }

    auto method = it->second;
    self.methods_.erase(it);

    std::erase(self.ordered_names_, name);

    return method;
  }

  auto rename(this auto& self,
              const std::string& old_name,
              const std::string& new_name)
   -> std::expected<void, std::system_error>
  {
    if(!self.has(old_name)) {
      return std::unexpected{std::system_error{
       std::make_error_code(std::errc::invalid_argument),
       std::format("Cannot change name of non-existing method '{}'",
                   old_name)}};
    }

    if(old_name != new_name) {
      auto order_it = std::ranges::find(self.ordered_names_, old_name);
      auto node = self.methods_.extract(old_name);
      if(node.empty() || order_it == self.ordered_names_.end()) {
        return std::unexpected{std::system_error{
         std::make_error_code(std::errc::identifier_removed),
         std::format("Failed to extract method with name '{}'", old_name)}};
      }

      auto new_it = std::ranges::find(self.ordered_names_, new_name);
      if(self.methods_.contains(new_name)) {
        std::erase(self.ordered_names_, new_name);
        self.methods_.extract(new_name);

        order_it = std::ranges::find(self.ordered_names_, old_name);
      }

      node.key() = new_name;
      self.methods_.insert(std::move(node));
      *order_it = new_name;

      assert(self.methods_.size() == self.ordered_names_.size());
    }

    return std::expected<void, std::system_error>{};
  }

  auto begin() noexcept -> Iterator
  {
    return Iterator{ordered_names_, methods_};
  }

  auto begin() const noexcept -> ConstIterator
  {
    return ConstIterator{ordered_names_, methods_};
  }

  auto end() const noexcept -> std::default_sentinel_t
  {
    return std::default_sentinel;
  }

  auto size(this const auto& self) noexcept -> size_t
  {
    assert(self.methods_.size() == self.ordered_names_.size());
    return self.methods_.size();
  }

  auto empty(this const auto& self) noexcept -> bool
  {
    assert(self.methods_.size() == self.ordered_names_.size());
    return self.methods_.empty();
  }

private:
  std::vector<std::string> ordered_names_;
  std::unordered_map<std::string, AnySeriesMethod> methods_;
};

} // namespace pludux