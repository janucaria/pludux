module;

#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <type_traits>

export module pludux:screener.any_method_context;

import :asset_snapshot;
import :screener.method_output;

export namespace pludux::screener {

class AnyMethodContext {
public:
  using DispatchResultType = double;

  AnyMethodContext()
  : impl_{nullptr}
  {
  }

  template<typename UImpl>
  AnyMethodContext(UImpl impl)
  : impl_{std::make_shared<ImplModel<UImpl>>(std::move(impl))}
  {
  }

  auto dispatch_call(this const auto& self,
                     const std::string& name,
                     AssetSnapshot asset_snapshot) noexcept
   -> DispatchResultType
  {
    return self.impl_
            ? self.impl_->dispatch_call(name, std::move(asset_snapshot))
            : std::numeric_limits<DispatchResultType>::quiet_NaN();
  }

  auto dispatch_call(this const auto& self,
                     const std::string& name,
                     AssetSnapshot asset_snapshot,
                     MethodOutput output_name) noexcept -> DispatchResultType
  {
    return self.impl_ ? self.impl_->dispatch_call(
                         name, std::move(asset_snapshot), output_name)
                      : std::numeric_limits<DispatchResultType>::quiet_NaN();
  }

  template<typename UImpl>
  friend auto method_context_cast(const AnyMethodContext& method) noexcept
   -> const UImpl*
  {
    auto model =
     std::dynamic_pointer_cast<const ImplModel<UImpl>>(method.impl_);
    return model ? &model->impl : nullptr;
  }

private:
  struct ImplConcept {
    virtual ~ImplConcept() = default;

    virtual auto dispatch_call(const std::string& name,
                               AssetSnapshot asset_snapshot) const noexcept
     -> DispatchResultType = 0;

    virtual auto dispatch_call(const std::string& name,
                               AssetSnapshot asset_snapshot,
                               MethodOutput output_name) const noexcept
     -> DispatchResultType = 0;
  };

  template<typename TImpl>
  struct ImplModel final : ImplConcept {
    TImpl impl;

    explicit ImplModel(TImpl impl)
    : impl{std::move(impl)}
    {
    }

    auto dispatch_call(const std::string& name,
                       AssetSnapshot asset_snapshot) const noexcept
     -> DispatchResultType override
    {
      return impl.dispatch_call(name, std::move(asset_snapshot));
    }

    auto dispatch_call(const std::string& name,
                       AssetSnapshot asset_snapshot,
                       MethodOutput output_name) const noexcept
     -> DispatchResultType override
    {
      return impl.dispatch_call(name, std::move(asset_snapshot), output_name);
    }
  };

  std::shared_ptr<const ImplConcept> impl_;
};

} // namespace pludux::screener
