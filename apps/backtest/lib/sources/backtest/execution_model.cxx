export module pludux.backtest:execution_model;

export namespace pludux::backtest {

enum class IntrabarPath { LowFirst, HighFirst, CandleDirection };

enum class ExitActivation { Simultaneous, AfterPrevious };

enum class SignalTiming { CurrentClose, NextOpen };

enum class PyramidingRetrigger { EveryEvaluation, AfterFalse };

} // namespace pludux::backtest
