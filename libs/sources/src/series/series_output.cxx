module;

#include <cstddef>
#include <limits>
#include <unordered_map>
#include <vector>

export module pludux.series.series_output;

export namespace pludux {

enum class OutputName {
  Default,
  UpperBand,
  MiddleBand,
  LowerBand,
  MacdLine,
  SignalLine,
  MacdHistogram,
  StochasticK,
  StochasticD
};

template<typename T>
class SeriesOutput {
public:
  using ValueType = T;

  SeriesOutput() = default;

  template<typename U>
    requires std::convertible_to<U, ValueType>
  SeriesOutput(U value)
  : SeriesOutput{{static_cast<ValueType>(value)}, {}}
  {
  }

  template<typename U>
    requires std::convertible_to<U, ValueType>
  SeriesOutput(const SeriesOutput<U>& other)
  : outputs_{}
  , output_name_to_index_{other.output_name_to_index()}
  {
    outputs_.reserve(other.size());
    for(const auto& v : other.outputs()) {
      outputs_.emplace_back(static_cast<ValueType>(v));
    }
  }

  SeriesOutput(std::vector<ValueType> outputs,
               std::unordered_map<OutputName, std::size_t> output_name_to_index)
  : outputs_{std::move(outputs)}
  , output_name_to_index_{std::move(output_name_to_index)}
  {
  }

  operator ValueType() const noexcept
  {
    return outputs_.empty() ? std::numeric_limits<ValueType>::quiet_NaN()
                            : outputs_[0];
  }

  auto outputs() const noexcept -> const std::vector<ValueType>&
  {
    return outputs_;
  }

  auto output_name_to_index() const noexcept
   -> const std::unordered_map<OutputName, std::size_t>&
  {
    return output_name_to_index_;
  }

  auto operator[](std::size_t index) const noexcept -> ValueType
  {
    return index < outputs_.size()
            ? outputs_[index]
            : std::numeric_limits<ValueType>::quiet_NaN();
  }

  auto size() const noexcept -> std::size_t
  {
    return outputs_.size();
  }

  auto get(OutputName name) const noexcept -> ValueType
  {
    const auto it = output_name_to_index_.find(name);
    if(it == output_name_to_index_.end()) {
      return std::numeric_limits<ValueType>::quiet_NaN();
    }

    return outputs_[it->second];
  }

private:
  std::vector<ValueType> outputs_;
  std::unordered_map<OutputName, std::size_t> output_name_to_index_;
};

} // namespace pludux
