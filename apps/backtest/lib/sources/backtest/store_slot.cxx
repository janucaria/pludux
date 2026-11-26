export module pludux.backtest:store_slot;

import :store_handle;

export namespace pludux::backtest {

class StoreSlot {
public:
  constexpr StoreSlot() noexcept = default;

  constexpr StoreSlot(StoreIndex value_index,
                      StoreGeneration generation,
                      bool alive) noexcept
  : value_index_{value_index}
  , generation_{generation}
  , alive_{alive}
  {
  }

  auto value_index(this const StoreSlot& self) noexcept -> StoreIndex
  {
    return self.value_index_;
  }

  auto generation(this const StoreSlot& self) noexcept -> StoreGeneration
  {
    return self.generation_;
  }

  auto alive(this const StoreSlot& self) noexcept -> bool
  {
    return self.alive_;
  }

  auto value_index(this StoreSlot& self, StoreIndex value_index) noexcept
   -> void
  {
    self.value_index_ = value_index;
  }

  auto generation(this StoreSlot& self, StoreGeneration generation) noexcept
   -> void
  {
    self.generation_ = generation;
  }

  auto alive(this StoreSlot& self, bool alive) noexcept -> void
  {
    self.alive_ = alive;
  }

private:
  StoreIndex value_index_{};
  StoreGeneration generation_{};
  bool alive_{};
};

} // namespace pludux::backtest
