export module pludux:methods.boolean_method;

export namespace pludux {

template<bool boolean_value>
struct BooleanMethod {
  auto operator==(const BooleanMethod&) const noexcept -> bool = default;
};

using TrueMethod = BooleanMethod<true>;
using FalseMethod = BooleanMethod<false>;

} // namespace pludux
