#pragma once

#include <string>
#include <zapata/applications/datums/Applications.h>
#include <zapata/applications/datums/MyApplications.h>
#include <zapata/applications/datums/MyUsers.h>
#include <zapata/applications/datums/ResourceOwners.h>
#include <zapata/mongodb.h>
#include <zapata/mysql.h>
#include <zapata/postgresql.h>
#include <zapata/redis.h>
#include <zapata/rest.h>

namespace zpt {
namespace apps {

namespace datums {
namespace MyApplications {
auto
get(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope)
  -> zpt::json;
auto
query(std::string _topic,
      zpt::json _filter,
      zpt::ev::emitter _emitter,
      zpt::json _identity,
      zpt::json _envelope) -> zpt::json;
auto
insert(std::string _topic,
       zpt::json _document,
       zpt::ev::emitter _emitter,
       zpt::json _identity,
       zpt::json _envelope) -> zpt::json;
auto
save(std::string _topic,
     zpt::json _document,
     zpt::ev::emitter _emitter,
     zpt::json _identity,
     zpt::json _envelope) -> zpt::json;
auto
set(std::string _topic,
    zpt::json _document,
    zpt::ev::emitter _emitter,
    zpt::json _identity,
    zpt::json _envelope) -> zpt::json;
auto
set(std::string _topic,
    zpt::json _document,
    zpt::json _filter,
    zpt::ev::emitter _emitter,
    zpt::json _identity,
    zpt::json _envelope) -> zpt::json;
auto
remove(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope)
  -> zpt::json;
auto
remove(std::string _topic,
       zpt::json _filter,
       zpt::ev::emitter _emitter,
       zpt::json _identity,
       zpt::json _envelope) -> zpt::json;
} // namespace MyApplications
} // namespace datums
} // namespace apps
} // namespace zpt
