module;

#include <functional>
#include <utility>

export module pludux:methods.comparison_method;

export namespace pludux {

template<typename TComparator,
         typename TTargetMethod,
         typename TThresholdMethod>
class ComparisonMethod {
public:
  ComparisonMethod(TTargetMethod target, TThresholdMethod threshold)
  : target_{std::move(target)}
  , threshold_{std::move(threshold)}
  {
  }

  auto operator==(const ComparisonMethod& other) const noexcept
   -> bool = default;

  auto target(this const ComparisonMethod& self) noexcept
   -> const TTargetMethod&
  {
    return self.target_;
  }

  void target(this ComparisonMethod& self, TTargetMethod target) noexcept
  {
    self.target_ = std::move(target);
  }

  auto threshold(this const ComparisonMethod& self) noexcept
   -> const TThresholdMethod&
  {
    return self.threshold_;
  }

  void threshold(this ComparisonMethod& self,
                 TThresholdMethod threshold) noexcept
  {
    self.threshold_ = std::move(threshold);
  }

private:
  TTargetMethod target_;
  TThresholdMethod threshold_;
};

template<typename TTargetMethod, typename TThresholdMethod>
using GreaterEqualMethod =
 ComparisonMethod<std::greater_equal<>, TTargetMethod, TThresholdMethod>;

template<typename TTargetMethod, typename TThresholdMethod>
using GreaterThanMethod =
 ComparisonMethod<std::greater<>, TTargetMethod, TThresholdMethod>;

template<typename TTargetMethod, typename TThresholdMethod>
using LessThanMethod =
 ComparisonMethod<std::less<>, TTargetMethod, TThresholdMethod>;

template<typename TTargetMethod, typename TThresholdMethod>
using LessEqualMethod =
 ComparisonMethod<std::less_equal<>, TTargetMethod, TThresholdMethod>;

template<typename TTargetMethod, typename TThresholdMethod>
using EqualMethod =
 ComparisonMethod<std::equal_to<>, TTargetMethod, TThresholdMethod>;

template<typename TTargetMethod, typename TThresholdMethod>
using NotEqualMethod =
 ComparisonMethod<std::not_equal_to<>, TTargetMethod, TThresholdMethod>;

template<typename TMethod>
class AboveByMethod {
public:
  AboveByMethod(TMethod target,
                TMethod reference,
                TMethod amount,
                bool by_percent)
  : target_{std::move(target)}
  , reference_{std::move(reference)}
  , amount_{std::move(amount)}
  , by_percent_{by_percent}
  {
  }

  auto operator==(const AboveByMethod&) const -> bool = default;

  auto target() const noexcept -> const TMethod&
  {
    return target_;
  }
  auto reference() const noexcept -> const TMethod&
  {
    return reference_;
  }
  auto amount() const noexcept -> const TMethod&
  {
    return amount_;
  }
  auto by_percent() const noexcept -> bool
  {
    return by_percent_;
  }

private:
  TMethod target_;
  TMethod reference_;
  TMethod amount_;
  bool by_percent_;
};

} // namespace pludux
