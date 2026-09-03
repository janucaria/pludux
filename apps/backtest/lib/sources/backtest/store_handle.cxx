module;

#include <cstddef>
#include <limits>

export module pludux.backtest:store_handle;

import pludux;

export namespace pludux::backtest {

template<typename TValue>
class StoreHandle {
public:
  static constexpr auto invalid_slot_index =
   std::numeric_limits<std::size_t>::max();

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

  auto valid(this const StoreHandle& self) noexcept -> bool
  {
    return self.slot_index_ != invalid_slot_index;
  }

  friend auto operator==(StoreHandle const&, StoreHandle const&)
   -> bool = default;

private:
  std::size_t slot_index_{invalid_slot_index};
  std::size_t generation_{};
};

struct SystemStoreHandle : StoreHandle<SystemStoreHandle> {
  using StoreHandle<SystemStoreHandle>::StoreHandle;
};

struct PortfolioStoreHandle : StoreHandle<PortfolioStoreHandle> {
  using StoreHandle<PortfolioStoreHandle>::StoreHandle;
};

struct AssetStoreHandle : StoreHandle<AssetStoreHandle> {
  using StoreHandle<AssetStoreHandle>::StoreHandle;
};

struct WatchlistStoreHandle : StoreHandle<WatchlistStoreHandle> {
  using StoreHandle<WatchlistStoreHandle>::StoreHandle;
};

struct ModelStoreHandle : StoreHandle<ModelStoreHandle> {
  using StoreHandle<ModelStoreHandle>::StoreHandle;
};

struct MarketStoreHandle : StoreHandle<MarketStoreHandle> {
  using StoreHandle<MarketStoreHandle>::StoreHandle;
};

struct BrokerStoreHandle : StoreHandle<BrokerStoreHandle> {
  using StoreHandle<BrokerStoreHandle>::StoreHandle;
};

struct ProfileStoreHandle : StoreHandle<ProfileStoreHandle> {
  using StoreHandle<ProfileStoreHandle>::StoreHandle;
};

struct StrategyStoreHandle : StoreHandle<StrategyStoreHandle> {
  using StoreHandle<StrategyStoreHandle>::StoreHandle;
};

} // namespace pludux::backtest
