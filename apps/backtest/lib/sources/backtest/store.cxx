module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iterator>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

#include <stdexcept>

export module pludux.backtest:store;

import pludux;

export namespace pludux::backtest {

template<typename TValue>
class StoreHandle {
public:
  constexpr StoreHandle() noexcept = default;

  constexpr StoreHandle(std::size_t slot_index, std::size_t generation) noexcept
  : slot_index_{slot_index}
  , generation_{generation}
  {
  }

  auto slot_index(this const StoreHandle& self) noexcept -> std::size_t
  {
    return self.slot_index_;
  }

  auto generation(this const StoreHandle& self) noexcept -> std::size_t
  {
    return self.generation_;
  }

  friend auto operator==(StoreHandle const&, StoreHandle const&)
   -> bool = default;

private:
  std::size_t slot_index_{};
  std::size_t generation_{};
};

class StoreSlot {
public:
  constexpr StoreSlot() noexcept = default;

  constexpr StoreSlot(std::size_t value_index,
                      std::size_t generation,
                      bool alive) noexcept
  : value_index_{value_index}
  , generation_{generation}
  , alive_{alive}
  {
  }

  auto value_index(this const StoreSlot& self) noexcept -> std::size_t
  {
    return self.value_index_;
  }

  auto generation(this const StoreSlot& self) noexcept -> std::size_t
  {
    return self.generation_;
  }

  auto alive(this const StoreSlot& self) noexcept -> bool
  {
    return self.alive_;
  }

  auto set_value_index(this StoreSlot& self, std::size_t value_index) noexcept
   -> void
  {
    self.value_index_ = value_index;
  }

  auto set_generation(this StoreSlot& self, std::size_t generation) noexcept
   -> void
  {
    self.generation_ = generation;
  }

  auto set_alive(this StoreSlot& self, bool alive) noexcept -> void
  {
    self.alive_ = alive;
  }

private:
  std::size_t value_index_{};
  std::size_t generation_{};
  bool alive_{};
};

template<typename TValue, typename THandleValue = TValue>
class Store {
public:
  using value_type = TValue;
  using handle_type = StoreHandle<THandleValue>;
  using slot_type = StoreSlot;

  Store() = default;

  Store(std::vector<TValue> values,
        std::vector<StoreSlot> slots,
        std::vector<std::size_t> value_to_slot_indices,
        std::vector<std::size_t> free_slot_indices)
  : values_{std::move(values)}
  , slots_{std::move(slots)}
  , value_to_slot_indices_{std::move(value_to_slot_indices)}
  , free_slot_indices_{std::move(free_slot_indices)}
  {
  }

  auto values(this const Store& self) noexcept -> const std::vector<TValue>&
  {
    return self.values_;
  }

  auto slots(this const Store& self) noexcept -> const std::vector<StoreSlot>&
  {
    return self.slots_;
  }

  auto value_to_slot_indices(this const Store& self) noexcept
   -> const std::vector<std::size_t>&
  {
    return self.value_to_slot_indices_;
  }

  auto free_slot_indices(this const Store& self) noexcept
   -> const std::vector<std::size_t>&
  {
    return self.free_slot_indices_;
  }

  auto size(this const Store& self) noexcept -> std::size_t
  {
    return self.values_.size();
  }

  auto empty(this const Store& self) noexcept -> bool
  {
    return self.values_.empty();
  }

  auto is_alive(this const Store& self, handle_type handle) noexcept -> bool
  {
    const auto slot_index = handle.slot_index();
    if(slot_index >= self.slots_.size()) {
      return false;
    }

    const auto& slot = self.slots_[slot_index];
    return slot.alive() && slot.generation() == handle.generation();
  }

  auto add(this Store& self, TValue value) -> handle_type
  {
    const auto value_index = self.values_.size();

    if(!self.free_slot_indices_.empty()) {
      const auto slot_index = self.free_slot_indices_.back();
      self.free_slot_indices_.pop_back();

      auto& slot = self.slots_[slot_index];
      slot.set_value_index(value_index);
      slot.set_alive(true);

      self.values_.push_back(std::move(value));
      self.value_to_slot_indices_.push_back(slot_index);

      return handle_type{slot_index, slot.generation()};
    }

    const auto slot_index = self.slots_.size();

    self.slots_.push_back(StoreSlot{value_index, 0, true});

    self.values_.push_back(std::move(value));
    self.value_to_slot_indices_.push_back(slot_index);

    return handle_type{slot_index, 0};
  }

  auto add(this Store& self, handle_type handle, TValue value) -> bool
  {
    const auto slot_index = handle.slot_index();
    if(slot_index >= self.slots_.size()) {
      for(std::size_t i = self.slots_.size(); i <= slot_index; ++i) {
        self.slots_.push_back(StoreSlot{});
        self.free_slot_indices_.push_back(i);
      }
    }

    // check if the slot is a free slot that can be used
    auto free_slot_it = std::ranges::find(self.free_slot_indices_, slot_index);
    if(free_slot_it == self.free_slot_indices_.end()) {
      return false;
    }

    self.free_slot_indices_.erase(free_slot_it);

    auto& slot = self.slots_[slot_index];
    slot.set_value_index(self.values_.size());
    slot.set_generation(handle.generation());
    slot.set_alive(true);

    self.values_.push_back(std::move(value));
    self.value_to_slot_indices_.push_back(slot_index);

    return true;
  }

  auto remove(this Store& self, handle_type handle) -> bool
  {
    const auto slot_index = handle.slot_index();
    if(slot_index >= self.slots_.size()) {
      return false;
    }

    auto& removed_slot = self.slots_[slot_index];
    if(!removed_slot.alive() ||
       removed_slot.generation() != handle.generation()) {
      return false;
    }

    assert(!self.values_.empty());

    const auto removed_value_index = removed_slot.value_index();
    const auto last_value_index = self.values_.size() - 1;

    assert(removed_value_index < self.values_.size());
    assert(last_value_index < self.value_to_slot_indices_.size());

    if(removed_value_index != last_value_index) {
      std::swap(self.values_[removed_value_index],
                self.values_[last_value_index]);

      const auto moved_slot_index =
       self.value_to_slot_indices_[last_value_index];

      self.value_to_slot_indices_[removed_value_index] = moved_slot_index;
      self.slots_[moved_slot_index].set_value_index(removed_value_index);
    }

    self.values_.pop_back();
    self.value_to_slot_indices_.pop_back();

    removed_slot.set_alive(false);
    removed_slot.set_generation(removed_slot.generation() + 1);

    self.free_slot_indices_.push_back(slot_index);

    return true;
  }

  auto update(this Store& self, handle_type handle, TValue new_value) -> bool
  {
    const auto slot_index = handle.slot_index();
    if(slot_index >= self.slots_.size()) {
      return false;
    }

    auto& slot = self.slots_[slot_index];
    if(!slot.alive() || slot.generation() != handle.generation()) {
      return false;
    }

    const auto value_index = slot.value_index();
    assert(value_index < self.values_.size());

    self.values_[value_index] = std::move(new_value);

    return true;
  }

  auto get_if_present(this const Store& self, handle_type handle)
   -> const TValue*
  {
    const auto slot_index = handle.slot_index();
    if(slot_index >= self.slots_.size()) {
      return nullptr;
    }

    const auto& slot = self.slots_[slot_index];
    if(!slot.alive() || slot.generation() != handle.generation()) {
      return nullptr;
    }

    return &self.values_[slot.value_index()];
  }

  auto get_if_present(this Store& self, handle_type handle) -> TValue*
  {
    const auto slot_index = handle.slot_index();
    if(slot_index >= self.slots_.size()) {
      return nullptr;
    }

    auto& slot = self.slots_[slot_index];
    if(!slot.alive() || slot.generation() != handle.generation()) {
      return nullptr;
    }

    return &self.values_[slot.value_index()];
  }

  auto get(this const Store& self, handle_type handle) -> const TValue&
  {
    auto value = self.get_if_present(handle);
    if(value == nullptr) {
      throw std::out_of_range{
       "Handle does not refer to a valid value in the store."};
    }
    assert(value != nullptr);
    return *value;
  }

  auto get(this Store& self, handle_type handle) -> TValue&
  {
    auto value = self.get_if_present(handle);
    if(value == nullptr) {
      throw std::out_of_range{
       "Handle does not refer to a valid value in the store."};
    }
    assert(value != nullptr);
    return *value;
  }

private:
  std::vector<TValue> values_;
  std::vector<StoreSlot> slots_;
  std::vector<std::size_t> value_to_slot_indices_;
  std::vector<std::size_t> free_slot_indices_;
};

} // namespace pludux::backtest
