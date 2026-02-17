#pragma once

#include <zapata/http.h>
#include <zapata/ontology/message.h>

namespace zpt {
namespace http {
auto retrieve(zpt::message _to_send) -> zpt::message;
auto resolve(std::string const& _domain) -> zpt::json;
} // namespace http
} // namespace zpt
