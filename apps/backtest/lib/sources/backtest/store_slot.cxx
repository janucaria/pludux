module;

#include <cstddef>

export module pludux.backtest:store_slot;

import pludux;

export namespace pludux::backtest {

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

  auto value_index(this StoreSlot& self, std::size_t value_index) noexcept
   -> void
  {
    self.value_index_ = value_index;
  }

  auto generation(this StoreSlot& self, std::size_t generation) noexcept
   -> void
  {
    self.generation_ = generation;
  }

  auto alive(this StoreSlot& self, bool alive) noexcept -> void
  {
    self.alive_ = alive;
  }

private:
  std::size_t value_index_{};
  std::size_t generation_{};
  bool alive_{};
};

} // namespace pludux::backtest
