#pragma once

#include <zapata/json/JSONClass.h>

namespace zpt {
namespace uri {
auto parse(std::string const& _in, zpt::JSONType _type = zpt::JSObject) -> zpt::json;
auto parse(std::istream& _in, zpt::JSONType _type = zpt::JSObject) -> zpt::json;
auto to_string(zpt::json const& _uri) -> std::string;
auto to_regex(zpt::json const& _in) -> zpt::json;
auto to_regex_object(zpt::json const& _in) -> zpt::json;
auto to_regex_array(zpt::json const& _in) -> zpt::json;
namespace path {
auto to_string(zpt::json const& _uri) -> std::string;
} // namespace path
namespace address {
auto to_string(zpt::json const& _uri) -> std::string;
} // namespace address
namespace params {
auto to_string(zpt::json const& _uri) -> std::string;
} // namespace params
} // namespace uri
} // namespace zpt
