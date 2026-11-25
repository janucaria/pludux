module;

#include <cstddef>
#include <format>
#include <istream>
#include <stdexcept>
#include <string>
#include <utility>

#include <rapidcsv.h>

export module pludux.backtest:call_csv_import;

import :calls;

export namespace pludux::backtest {

auto read_call_csv(std::istream& input, std::string name) -> CallTable
{
  const auto csv =
   rapidcsv::Document{input,
                      rapidcsv::LabelParams{0, -1},
                      rapidcsv::SeparatorParams{',', false, false, true}};
  auto table = CallTable{std::move(name), csv.GetColumnNames(), {}};
  if(table.headers.empty())
    throw std::invalid_argument{"A Calls CSV file requires a header row"};
  table.rows.reserve(csv.GetRowCount());
  for(std::size_t i = 0; i < csv.GetRowCount(); ++i) {
    auto row = csv.GetRow<std::string>(i);
    if(row.size() != table.headers.size())
      throw std::invalid_argument{
       std::format("CSV row {} has the wrong number of columns", i + 2)};
    table.rows.push_back(std::move(row));
  }
  return table;
}

} // namespace pludux::backtest
