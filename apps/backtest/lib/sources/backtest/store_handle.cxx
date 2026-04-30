module;

#include <cstddef>

export module pludux.backtest:store_handle;

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

struct BacktestStoreHandle : StoreHandle<BacktestStoreHandle> {
  using StoreHandle<BacktestStoreHandle>::StoreHandle;
};

struct AssetStoreHandle : StoreHandle<AssetStoreHandle> {
  using StoreHandle<AssetStoreHandle>::StoreHandle;
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
