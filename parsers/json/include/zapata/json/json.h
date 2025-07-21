#pragma once

#include <filesystem>
#include <string>
#include <unistd.h>
#include <zapata/json/JSONClass.h>
#include <zapata/json/JSONParser.h>

namespace zpt {

auto to_string(zpt::json _in) -> std::string;

auto split(std::string const& _to_split, std::string const& _separator, bool _trim = false)
  -> zpt::json;
auto join(zpt::json _to_join, std::string const& _separator) -> std::string;

namespace path {
auto split(std::string const& _to_split) -> zpt::json;
auto join(zpt::json _to_join) -> std::string;
} // namespace path

namespace email {
auto parse(std::string const& _email) -> zpt::json;
}

auto to_str(zpt::json _uri, zpt::json _opts = zpt::undefined) -> std::string;

namespace conf {
auto getopt(int _argc, char* _argv[]) -> zpt::json;
auto setup(zpt::json _options) -> void;
auto evaluate_ref(zpt::json _options,
                  zpt::json _parent,
                  std::variant<std::string, size_t> const& _parent_key,
                  std::filesystem::path const& _context,
                  zpt::json _root) -> void;
auto file(std::filesystem::path const& _file, zpt::json& _options, zpt::json _root) -> void;
auto dirs(std::string const& _dir, zpt::json& _options) -> void;
auto dirs(zpt::json& _options) -> void;
auto env(zpt::json& _options) -> void;
} // namespace conf

namespace parameters {
auto parse(int _argc, char* _argv[], zpt::json _config) -> zpt::json;
auto verify(zpt::json _to_check, zpt::json _rules) -> void;
auto usage(zpt::json _config) -> std::string;
} // namespace parameters

namespace test {
auto location(zpt::json _location) -> bool;
auto timestamp(zpt::json _timestamp) -> bool;
} // namespace test

namespace http {
namespace cookies {
auto deserialize(std::string const& _cookie_header) -> zpt::json;
auto serialize(zpt::json _info) -> std::string;
} // namespace cookies
} // namespace http
} // namespace zpt
