module;

export module pludux:series_output;

export namespace pludux {

enum class SeriesOutput {
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