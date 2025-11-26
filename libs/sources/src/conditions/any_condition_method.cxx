module;

#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

export module pludux:conditions.any_condition_method;

import :asset_snapshot;
import :any_method_context;

export namespace pludux {

class AnyConditionMethod {
public:
  template<typename UImpl>
    requires std::
     is_invocable_r_v<bool, UImpl, AssetSnapshot, AnySeriesMethodContext>
   AnyConditionMethod(UImpl impl)
  : impl_{std::make_shared<ImplModel<UImpl>>(std::move(impl))}
  {
  }

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  AnySeriesMethodContext context) noexcept -> bool
  {
    return self.impl_->operator()(std::move(asset_snapshot), context);
  }

  auto operator==(this const auto& self,
                  const AnyConditionMethod& other) noexcept -> bool
  {
    return self.impl_->operator==(other);
  }

  auto operator!=(this const auto& self,
                  const AnyConditionMethod& other) noexcept -> bool
  {
    return self.impl_->operator!=(other);
  }

  template<typename UImpl>
  friend auto condition_method_cast(const AnyConditionMethod& method) noexcept
   -> const UImpl*
  {
    auto model =
     std::dynamic_pointer_cast<const ImplModel<UImpl>>(method.impl_);
    return model ? &model->impl : nullptr;
  }

private:
  struct ImplConcept {
    virtual ~ImplConcept() = default;

    virtual auto operator()(AssetSnapshot asset_snapshot,
                            AnySeriesMethodContext context) const noexcept
     -> bool = 0;

    virtual auto operator==(const AnyConditionMethod& other) const noexcept
     -> bool = 0;

    virtual auto operator!=(const AnyConditionMethod& other) const noexcept
     -> bool = 0;
  };

  template<typename TImpl>
  struct ImplModel final : ImplConcept {
    TImpl impl;

    explicit ImplModel(TImpl other_impl)
    : impl(std::move(other_impl))
    {
    }

    auto operator()(AssetSnapshot asset_snapshot,
                    AnySeriesMethodContext context) const noexcept
     -> bool override
    {
      return impl(std::move(asset_snapshot), context);
    }

    auto operator==(const AnyConditionMethod& other) const noexcept
     -> bool override
    {
      auto other_model = condition_method_cast<TImpl>(other);
      return other_model && impl == *other_model;
    }

    auto operator!=(const AnyConditionMethod& other) const noexcept
     -> bool override
    {
      auto other_model = condition_method_cast<TImpl>(other);
      return !other_model || impl != *other_model;
    }
  };

  std::shared_ptr<const ImplConcept> impl_;
};

} // namespace pludux
