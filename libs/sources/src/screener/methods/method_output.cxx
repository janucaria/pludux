module;

export module pludux:screener.method_output;

export namespace pludux::screener {

enum class MethodOutput {
  UpperBand,
  MiddleBand,
  LowerBand,
  MacdLine,
  SignalLine,
  Histogram,
  KPercent,
  DPercent
};

} // namespace pludux::screener