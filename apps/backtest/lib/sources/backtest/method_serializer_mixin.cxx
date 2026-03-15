module;

#include <functional>
#include <stdexcept>
#include <string>
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
    return parameters.contains(key) ? parameters.at(key).as<TMethod>()
                                    : default_value;
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
    self.method_parsers_.emplace(
     method_name, std::make_pair(method_serialize, method_deserialize));
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

    for(const auto& [method_name, method_parser] : self.method_parsers_) {
      const auto& [method_params_serialize, _] = method_parser;
      auto serialized_params_method = method_params_serialize(derived, method);
      if(!serialized_params_method.is_null()) {
        auto serialized_method = jsoncons::ojson{};
        serialized_method["method"] = method_name;
        if(!serialized_params_method.empty()) {
          serialized_method["params"] = std::move(serialized_params_method);
        }

        return serialized_method;
      }
    }

    return jsoncons::ojson::null();
  }

private:
  std::unordered_map<std::string, std::pair<MethodSerialize, MethodDeserialize>>
   method_parsers_;
};

} // namespace pludux::backtest
