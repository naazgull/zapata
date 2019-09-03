#pragma once

#include <string>
#include <zapata/applications/datums/Applications.h>
#include <zapata/applications/datums/MyApplications.h>
#include <zapata/applications/datums/MyUsers.h>
#include <zapata/applications/datums/ResourceOwners.h>
#include <zapata/applications/mutations/Applications.h>
#include <zapata/applications/mutations/MyApplications.h>
#include <zapata/applications/mutations/MyUsers.h>
#include <zapata/applications/mutations/ResourceOwners.h>
#include <zapata/rest.h>

namespace zpt {
namespace apps {
namespace documents {
namespace applications {

auto
restify(zpt::ev::emitter _emitter) -> void;
}
} // namespace documents
} // namespace apps
} // namespace zpt
