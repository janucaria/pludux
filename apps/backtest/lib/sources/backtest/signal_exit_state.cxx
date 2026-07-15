module;

export module pludux.backtest:signal_exit_state;

export namespace pludux::backtest {

class SignalExitState {
public:
  SignalExitState() = default;

  SignalExitState(bool enabled, bool consumed = false) noexcept
  : enabled_{enabled}
  , consumed_{consumed}
  {
  }

  auto operator==(const SignalExitState&) const noexcept -> bool = default;

  auto enabled(this const SignalExitState& self) noexcept -> bool
  {
    return self.enabled_;
  }

  auto consumed(this const SignalExitState& self) noexcept -> bool
  {
    return self.consumed_;
  }

  void consumed(this SignalExitState& self, bool consumed) noexcept
  {
    self.consumed_ = consumed;
  }

  auto active(this const SignalExitState& self) noexcept -> bool
  {
    return self.enabled_ && !self.consumed_;
  }

private:
  bool enabled_{};
  bool consumed_{};
};

} // namespace pludux::backtest
