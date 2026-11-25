module;

#include <charconv>
#include <chrono>
#include <cmath>
#include <ctime>
#include <format>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

#include <ctre.hpp>

export module pludux.backtest:table_field_parsing;

export namespace pludux::backtest {

auto parse_finite_number(std::string_view text) -> double
{
  if(text.starts_with('+')) {
    text.remove_prefix(1);
    if(text.starts_with('-'))
      throw std::invalid_argument{"Expected a finite number"};
  }
  if(text.empty())
    throw std::invalid_argument{"Expected a finite number"};
  double value{};
  const auto end = text.data() + text.size();
  const auto parsed = std::from_chars(text.data(), end, value);
  if(parsed.ec != std::errc{} || parsed.ptr != end || !std::isfinite(value))
    throw std::invalid_argument{"Expected a finite number"};
  return value;
}

auto parse_candle_datetime(std::string_view text) -> double
{
  const auto first = text.find_first_not_of(" \t\r\n");
  if(first == text.npos)
    throw std::invalid_argument{"Expected a datetime"};
  text = text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);
  const auto match = ctre::match<
   R"(^(\d{4})-(\d{2})-(\d{2})(?:[T ](\d{2}):(\d{2})(?::(\d{2})(\.\d+)?)?(Z|[+\-]\d{2}:\d{2})?)?$)">(
   text);
  if(!match)
    return parse_finite_number(text);

  const auto number = [](auto component) {
    return component ? std::stoi(component.str()) : 0;
  };
  const auto year = number(match.get<1>());
  const auto month = number(match.get<2>());
  const auto day = number(match.get<3>());
  const auto hour = number(match.get<4>());
  const auto minute = number(match.get<5>());
  const auto second = number(match.get<6>());
  const auto fraction =
   match.get<7>() ? parse_finite_number(match.get<7>().to_view()) : 0.;
  const auto date = std::chrono::year{year} / month / day;
  if(!date.ok() || hour > 23 || minute > 59 || second > 59)
    throw std::invalid_argument{"Invalid calendar datetime"};

  // Preserve the Asset importer's platform-local whole-second conversion.
  const auto timezone = match.get<8>().to_optional_string().value_or("Z");
  auto input =
   std::istringstream{std::format("{:04}-{:02}-{:02}T{:02}:{:02}:{:02}{}",
                                  year,
                                  month,
                                  day,
                                  hour,
                                  minute,
                                  second,
                                  timezone)};
  std::tm local{};
  input >> std::get_time(&local, "%Y-%m-%dT%H:%M:%S%z");
  const auto time = std::mktime(&local);
  if(time == static_cast<std::time_t>(-1))
    throw std::invalid_argument{"Datetime outside supported range"};
  return static_cast<double>(time) + fraction;
}

} // namespace pludux::backtest
