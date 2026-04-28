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

export module pludux.backtest:store_data_resolver;

import pludux;

import :store_handle;
import :store_slot;

export namespace pludux::backtest {

template<typename TValue, typename THandle>
class StoreDataResolver {
public:
  using ValueType = TValue;
  using HandleType = THandle;

  StoreDataResolver() = default;

  StoreDataResolver(std::vector<StoreSlot> slots,
                    std::vector<std::size_t> value_to_slot_indices,
                    std::vector<std::size_t> free_slot_indices)
  : slots_{std::move(slots)}
  , value_to_slot_indices_{std::move(value_to_slot_indices)}
  , free_slot_indices_{std::move(free_slot_indices)}
  {
  }

  auto slots(this const StoreDataResolver& self) noexcept
   -> const std::vector<StoreSlot>&
  {
    return self.slots_;
  }

  auto value_to_slot_indices(this const StoreDataResolver& self) noexcept
   -> const std::vector<std::size_t>&
  {
    return self.value_to_slot_indices_;
  }

  auto free_slot_indices(this const StoreDataResolver& self) noexcept
   -> const std::vector<std::size_t>&
  {
    return self.free_slot_indices_;
  }

  auto is_alive(this const StoreDataResolver& self, HandleType handle) noexcept
   -> bool
  {
    const auto slot_index = handle.slot_index();
    if(slot_index >= self.slots_.size()) {
      return false;
    }

    const auto& slot = self.slots_[slot_index];
    return slot.alive() && slot.generation() == handle.generation();
  }

  auto add(this StoreDataResolver& self,
           std::vector<ValueType>& values,
           ValueType value) -> HandleType
  {
    const auto value_index = values.size();

    if(!self.free_slot_indices_.empty()) {
      const auto slot_index = self.free_slot_indices_.back();
      self.free_slot_indices_.pop_back();

      auto& slot = self.slots_[slot_index];
      slot.value_index(value_index);
      slot.alive(true);

      values.push_back(std::move(value));
      self.value_to_slot_indices_.push_back(slot_index);

      return HandleType{slot_index, slot.generation()};
    }

    const auto slot_index = self.slots_.size();

    self.slots_.push_back(StoreSlot{value_index, 0, true});

    values.push_back(std::move(value));
    self.value_to_slot_indices_.push_back(slot_index);

    return HandleType{slot_index, 0};
  }

  auto add(this StoreDataResolver& self,
           std::vector<ValueType>& values,
           HandleType handle,
           ValueType value) -> bool
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
    slot.value_index(values.size());
    slot.generation(handle.generation());
    slot.alive(true);

    values.push_back(std::move(value));
    self.value_to_slot_indices_.push_back(slot_index);

    return true;
  }

  auto remove(this StoreDataResolver& self,
              std::vector<ValueType>& values,
              HandleType handle) -> bool
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

    assert(!values.empty());

    const auto removed_value_index = removed_slot.value_index();
    const auto last_value_index = values.size() - 1;

    assert(removed_value_index < values.size());
    assert(last_value_index < self.value_to_slot_indices_.size());

    if(removed_value_index != last_value_index) {
      std::swap(values[removed_value_index], values[last_value_index]);

      const auto moved_slot_index =
       self.value_to_slot_indices_[last_value_index];

      self.value_to_slot_indices_[removed_value_index] = moved_slot_index;
      self.slots_[moved_slot_index].value_index(removed_value_index);
    }

    values.pop_back();
    self.value_to_slot_indices_.pop_back();

    removed_slot.generation(removed_slot.generation() + 1);
    removed_slot.alive(false);

    self.free_slot_indices_.push_back(slot_index);

    return true;
  }

  auto update(this StoreDataResolver& self,
              std::vector<ValueType>& values,
              HandleType handle,
              ValueType new_value) -> bool
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
    assert(value_index < values.size());

    values[value_index] = std::move(new_value);

    return true;
  }

  auto get_if_present(this const StoreDataResolver& self,
                      const std::vector<ValueType>& values,
                      HandleType handle) -> const ValueType*
  {
    const auto slot_index = handle.slot_index();
    if(slot_index >= self.slots_.size()) {
      return nullptr;
    }

    const auto& slot = self.slots_[slot_index];
    if(!slot.alive() || slot.generation() != handle.generation()) {
      return nullptr;
    }

    return &(values[slot.value_index()]);
  }

  auto get_if_present(this const StoreDataResolver& self,
                      std::vector<ValueType>& values,
                      HandleType handle) -> ValueType*
  {
    const auto slot_index = handle.slot_index();
    if(slot_index >= self.slots_.size()) {
      return nullptr;
    }

    auto& slot = self.slots_[slot_index];
    if(!slot.alive() || slot.generation() != handle.generation()) {
      return nullptr;
    }

    return &values[slot.value_index()];
  }

  auto get(this const StoreDataResolver& self,
           const std::vector<ValueType>& values,
           HandleType handle) -> const ValueType&
  {
    auto value = self.get_if_present(values, handle);
    assert(value != nullptr);
    return *value;
  }

  auto get(this const StoreDataResolver& self,
           std::vector<ValueType>& values,
           HandleType handle) -> ValueType&
  {
    auto value = self.get_if_present(values, handle);
    assert(value != nullptr);
    return *value;
  }

private:
  std::vector<StoreSlot> slots_;
  std::vector<std::size_t> value_to_slot_indices_;
  std::vector<std::size_t> free_slot_indices_;
};

} // namespace pludux::backtest
