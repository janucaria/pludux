module;

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <format>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

export module pludux.backtest:calls;

import :table_field_parsing;

export namespace pludux::backtest {

struct CallTable {
  std::string name;
  std::vector<std::string> headers;
  std::vector<std::vector<std::string>> rows;

  auto operator==(const CallTable&) const -> bool = default;
};

struct CallColumns {
  std::string datetime;
  std::string asset;
  std::string direction;
  std::string entry_low;
  std::string entry_high;

  auto operator==(const CallColumns&) const -> bool = default;
};

using CallMappings = std::map<std::string, std::string>;
enum class EntryPriority { CallFirst, SignalFirst };

struct CallRow {
  std::size_t row{};
  bool is_long{};
  std::optional<double> entry_low;
  std::optional<double> entry_high;
  std::map<std::string, double> fields;
};

using CallValues = std::map<double, CallRow>;

struct CompiledCalls {
  // Setup order, followed by Watchlist Asset order.
  std::vector<std::vector<CallValues>> values;
  std::vector<std::size_t> outside_watchlist;
};

auto compile_calls(const CallTable& table,
                   const CallColumns& columns,
                   const std::vector<std::string>& watchlist_names,
                   const std::vector<CallMappings>& setups) -> CompiledCalls
{
  if(table.headers.empty())
    throw std::invalid_argument{"A Call table requires column headers"};
  const auto column = [&](const std::string& header, std::string_view field) {
    const auto it = std::ranges::find(table.headers, header);
    if(header.empty() || it == table.headers.end() ||
       std::ranges::count(table.headers, header) != 1)
      throw std::invalid_argument{
       std::format("Select a unique Calls column for {}", field)};
    return static_cast<std::size_t>(it - table.headers.begin());
  };
  const auto datetime =
   columns.datetime.empty() ? 0 : column(columns.datetime, "Datetime");
  const auto asset = column(columns.asset, "Asset");

  const auto optional_column =
   [&](const std::string& name,
       std::string_view label) -> std::optional<std::size_t> {
    return name.empty() ? std::nullopt : std::optional{column(name, label)};
  };
  const auto direction = optional_column(columns.direction, "Direction");
  const auto low = optional_column(columns.entry_low, "Entry Low");
  const auto high = optional_column(columns.entry_high, "Entry High");

  using Binding = std::pair<std::size_t, std::string>;
  auto bindings = std::map<std::size_t, std::vector<Binding>>{};
  for(std::size_t i = 0; i < setups.size(); ++i)
    for(const auto& [field, header] : setups[i])
      bindings[column(header, field)].emplace_back(i, field);

  auto names = std::map<std::string, std::vector<std::size_t>>{};
  for(std::size_t index = 0; index < watchlist_names.size(); ++index)
    names[watchlist_names[index]].push_back(index);
  auto filenames = std::map<std::string, std::vector<std::size_t>>{};
  for(std::size_t index = 0; index < watchlist_names.size(); ++index) {
    const auto& name = watchlist_names[index];
    const auto filename = name.substr(name.find_last_of("/\\") + 1);
    if(filename.ends_with(".csv"))
      filenames[filename].push_back(index);
  }
  auto result = CompiledCalls{};
  result.values.resize(setups.size(),
                       std::vector<CallValues>(watchlist_names.size()));
  auto seen = std::map<std::pair<std::size_t, double>, std::size_t>{};
  const auto trim = [](std::string_view text) {
    const auto first = text.find_first_not_of(" \n\t\r");
    if(first == text.npos)
      return std::string_view{};
    return text.substr(first, text.find_last_not_of(" \n\t\r") - first + 1);
  };

  for(std::size_t i = 0; i < table.rows.size(); ++i) {
    const auto& row = table.rows[i];
    const auto row_number = i + 2; // The header is row 1.
    if(row.size() != table.headers.size())
      throw std::invalid_argument{std::format(
       "Call table row {} has the wrong number of columns", row_number)};
    const auto symbol = std::string{trim(row[asset])};
    if(symbol.empty())
      throw std::invalid_argument{
       std::format("Call table row {}, column '{}': blank Asset",
                   row_number,
                   table.headers[asset])};
    const auto exact = names.find(symbol);
    const auto filename =
     filenames.find(symbol.ends_with(".csv") ? symbol : symbol + ".csv");
    const auto* matches = exact != names.end()          ? &exact->second
                          : filename != filenames.end() ? &filename->second
                                                        : nullptr;
    if(!matches) {
      result.outside_watchlist.push_back(row_number);
      continue;
    }
    if(matches->size() != 1)
      throw std::invalid_argument{std::format(
       "Call table row {}, column '{}': ambiguous Watchlist Asset '{}'",
       row_number,
       table.headers[asset],
       symbol)};
    const auto asset_index = matches->front();
    double timestamp;
    try {
      timestamp = parse_candle_datetime(row[datetime]);
    } catch(const std::exception&) {
      throw std::invalid_argument{
       std::format("Call table row {}, column '{}': invalid Datetime '{}'",
                   row_number,
                   table.headers[datetime],
                   row[datetime])};
    }
    const auto [previous, unique] =
     seen.emplace(std::pair{asset_index, timestamp}, row_number);
    if(!unique)
      throw std::invalid_argument{std::format(
       "Call table rows {} and {} match the same Asset '{}' and Datetime",
       previous->second,
       row_number,
       watchlist_names[asset_index])};

    const auto numeric =
     [&](std::optional<std::size_t> index) -> std::optional<double> {
      if(!index || trim(row[*index]).empty())
        return std::nullopt;
      try {
        return parse_finite_number(trim(row[*index]));
      } catch(const std::exception&) {
        throw std::invalid_argument{
         std::format("Call table row {}, column '{}': expected a finite number",
                     row_number,
                     table.headers[*index])};
      }
    };
    auto side = direction ? std::string{trim(row[*direction])} : std::string{};
    std::ranges::transform(side, side.begin(), [](unsigned char ch) {
      return static_cast<char>(std::tolower(ch));
    });
    const auto is_long = side.empty() || side == "long" || side == "true" ||
                         side == "yes" || side == "1";
    const auto is_short =
     side == "short" || side == "false" || side == "no" || side == "0";
    if(!is_long && !is_short)
      throw std::invalid_argument{std::format(
       "Call table row {}, column '{}': expected Long, Short, True, "
       "False, Yes, No, 1, or 0",
       row_number,
       table.headers[*direction])};
    const auto call = CallRow{row_number, is_long, numeric(low), numeric(high)};
    if(call.entry_low && call.entry_high && *call.entry_low > *call.entry_high)
      throw std::invalid_argument{std::format(
       "Call table row {}, columns '{}' and '{}': Entry Low exceeds Entry High",
       row_number,
       columns.entry_low,
       columns.entry_high)};
    for(auto& setup : result.values)
      setup[asset_index].emplace(timestamp, call);
    for(const auto& [index, fields] : bindings) {
      if(const auto value = numeric(index)) {
        for(const auto& [setup, field] : fields)
          result.values[setup][asset_index].at(timestamp).fields.emplace(
           field, *value);
      }
    }
  }
  return result;
}

} // namespace pludux::backtest
