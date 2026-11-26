module;

#include <cstdint>
#include <limits>

export module pludux.backtest:store_handle;

import pludux;

export namespace pludux::backtest {

using StoreIndex = std::uint32_t;
using StoreGeneration = std::uint64_t;

template<typename TValue>
class StoreHandle {
public:
  static constexpr auto invalid_slot_index =
   std::numeric_limits<StoreIndex>::max();

  constexpr StoreHandle() noexcept = default;

  constexpr StoreHandle(StoreIndex slot_index,
                        StoreGeneration generation) noexcept
  : slot_index_{slot_index}
  , generation_{generation}
  {
  }

  auto slot_index(this const StoreHandle& self) noexcept -> StoreIndex
  {
    return self.slot_index_;
  }

  auto generation(this const StoreHandle& self) noexcept -> StoreGeneration
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
  StoreIndex slot_index_{invalid_slot_index};
  StoreGeneration generation_{};
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

struct CallStoreHandle : StoreHandle<CallStoreHandle> {
  using StoreHandle<CallStoreHandle>::StoreHandle;
};

struct WatchlistStoreHandle : StoreHandle<WatchlistStoreHandle> {
  using StoreHandle<WatchlistStoreHandle>::StoreHandle;
};

struct StrategyStoreHandle : StoreHandle<StrategyStoreHandle> {
  using StoreHandle<StrategyStoreHandle>::StoreHandle;
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


} // namespace pludux::backtest
