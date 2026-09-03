module;

#include <cmath>
#include <cstdint>
#include <format>
#include <functional>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <jsoncons/json.hpp>

export module pludux.backtest:method_serializer_mixin;

export namespace pludux::backtest {

template<typename TDerived, typename TAnyMethod>
class MethodSerializerMixin {
public:
  using MethodSerialize =
   std::function<auto(const TDerived&, const TAnyMethod&)->jsoncons::ojson>;

  using MethodDeserialize =
   std::function<auto(const TDerived&, const jsoncons::ojson&)->TAnyMethod>;

  template<typename TMethod>
  static auto get_param_or(const jsoncons::ojson& parameters,
                           const std::string& key,
                           const TMethod& default_value) -> TMethod
  {
    if(!parameters.contains(key)) {
      return default_value;
    }
    const auto& value = parameters.at(key);
    if constexpr(std::is_same_v<TMethod, bool>) {
      if(!value.is_bool()) {
        throw std::invalid_argument{"Expected a JSON boolean"};
      }
      return value.as_bool();
    } else if constexpr(std::is_same_v<TMethod, std::string>) {
      if(!value.is_string()) {
        throw std::invalid_argument{"Expected a JSON string"};
      }
      return value.as_string();
    } else if constexpr(std::is_floating_point_v<TMethod>) {
      if(!value.is_number()) {
        throw std::invalid_argument{"Expected a JSON number"};
      }
      const auto result = value.as<TMethod>();
      if(!std::isfinite(result)) {
        throw std::invalid_argument{"Expected a finite JSON number"};
      }
      return result;
    } else if constexpr(std::is_integral_v<TMethod>) {
      if(!value.is_number()) {
        throw std::invalid_argument{"Expected a JSON number"};
      }
      if(value.is_uint64()) {
        const auto result = value.as<std::uint64_t>();
        if constexpr(std::is_signed_v<TMethod>) {
          if(result >
             static_cast<std::uint64_t>(std::numeric_limits<TMethod>::max())) {
            throw std::out_of_range{"JSON integer is out of range"};
          }
        } else if(result > std::numeric_limits<TMethod>::max()) {
          throw std::out_of_range{"JSON integer is out of range"};
        }
        return static_cast<TMethod>(result);
      }
      if(value.is_int64()) {
        const auto result = value.as<std::int64_t>();
        if constexpr(std::is_unsigned_v<TMethod>) {
          if(result < 0 || static_cast<std::uint64_t>(result) >
                            std::numeric_limits<TMethod>::max()) {
            throw std::out_of_range{"JSON integer is out of range"};
          }
        } else if(result < static_cast<std::int64_t>(
                            std::numeric_limits<TMethod>::min()) ||
                  result > static_cast<std::int64_t>(
                            std::numeric_limits<TMethod>::max())) {
          throw std::out_of_range{"JSON integer is out of range"};
        }
        return static_cast<TMethod>(result);
      }
      const auto result = value.as_double();
      if(!std::isfinite(result) || std::trunc(result) != result) {
        throw std::invalid_argument{"Expected an integral JSON number"};
      }
      constexpr auto max_exact_integer = 9'007'199'254'740'991.0;
      if(std::abs(result) > max_exact_integer) {
        throw std::out_of_range{"Floating JSON integer is not lossless"};
      }
      if constexpr(std::is_unsigned_v<TMethod>) {
        if(result < 0.0 || static_cast<long double>(result) >
                            static_cast<long double>(
                             std::numeric_limits<TMethod>::max())) {
          throw std::out_of_range{"JSON integer is out of range"};
        }
      } else if(static_cast<long double>(result) <
                 static_cast<long double>(
                  std::numeric_limits<TMethod>::min()) ||
                static_cast<long double>(result) >
                 static_cast<long double>(
                  std::numeric_limits<TMethod>::max())) {
        throw std::out_of_range{"JSON integer is out of range"};
      }
      return static_cast<TMethod>(result);
    } else {
      return value.as<TMethod>();
    }
  }

  static auto parse_method_from_param_or(const TDerived& config_parser,
                                         const jsoncons::ojson& parameters,
                                         const std::string& key,
                                         const TAnyMethod& default_value)
   -> TAnyMethod
  {
    if(!parameters.contains(key)) {
      return default_value;
    }

    return config_parser.deserialize_method(parameters.at(key));
  }

  void register_method_parser(this MethodSerializerMixin& self,
                              const std::string& method_name,
                              const MethodSerialize& method_serialize,
                              const MethodDeserialize& method_deserialize)
  {
    const auto [_, inserted] = self.method_parsers_.emplace(
     method_name, std::make_pair(method_serialize, method_deserialize));
    if(!inserted) {
      throw std::invalid_argument{
       std::format("Method parser '{}' is already registered", method_name)};
    }
  }

  auto deserialize_method(this const MethodSerializerMixin& self,
                          const jsoncons::ojson& config_method) -> TAnyMethod
  {
    const auto& derived = static_cast<const TDerived&>(self);

    if(config_method.is_string()) {
      const auto named_method = config_method.as_string();
      const auto expanded_method =
       jsoncons::ojson::object{{"method", named_method}};
      return derived.deserialize_method(expanded_method);
    }

    if(!config_method.is_object()) {
      throw std::invalid_argument{"Expected a method object or shorthand string"};
    }

    if(!config_method.at("method").is_string()) {
      throw std::invalid_argument{"Expected method to be a string"};
    }
    const auto method = config_method.at("method").as_string();

    if(!self.method_parsers_.contains(method)) {
      const auto error_message = std::format("Unknown method: {}", method);
      throw std::invalid_argument{error_message};
    }

    try {
      const auto method_deserialize = self.method_parsers_.at(method).second;
      auto json_params = jsoncons::ojson{};

      json_params = config_method.contains("params")
                     ? config_method.at("params")
                     : jsoncons::ojson::object();
      if(!json_params.is_object()) {
        throw std::invalid_argument{"Expected method params to be an object"};
      }

      const auto method_result = method_deserialize(derived, json_params);

      return method_result;
    } catch(const std::exception& e) {
      const auto error_message =
       std::format("Error parsing method {}:\n{}", method, e.what());
      throw std::invalid_argument{error_message};
    } catch(...) {
      const auto error_message =
       std::format("Unknown error parsing method {}", method);
      throw std::invalid_argument{error_message};
    }
  }

  auto serialize_method(this const MethodSerializerMixin& self,
                        const TAnyMethod& method) -> jsoncons::ojson
  {
    const auto& derived = static_cast<const TDerived&>(self);

    auto serialized_method = jsoncons::ojson::null();
    auto matched_name = std::optional<std::string>{};
    for(const auto& [method_name, method_parser] : self.method_parsers_) {
      const auto& [method_params_serialize, _] = method_parser;
      auto serialized_params_method = method_params_serialize(derived, method);
      if(!serialized_params_method.is_null()) {
        if(matched_name) {
          throw std::invalid_argument{std::format(
           "Multiple method parsers match serialization: '{}' and '{}'",
           *matched_name,
           method_name)};
        }
        matched_name = method_name;
        serialized_method = jsoncons::ojson{};
        serialized_method["method"] = method_name;
        if(!serialized_params_method.empty()) {
          serialized_method["params"] = std::move(serialized_params_method);
        }
      }
    }

    return serialized_method;
  }

private:
  std::unordered_map<std::string, std::pair<MethodSerialize, MethodDeserialize>>
   method_parsers_;
};

} // namespace pludux::backtest
