module;

export module pludux:series.method_output;

export namespace pludux {

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

} // namespace pludux