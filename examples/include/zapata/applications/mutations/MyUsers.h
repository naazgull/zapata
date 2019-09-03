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
namespace mutations {
namespace MyUsers {

auto
mutify(zpt::mutation::emitter _emitter) -> void;
}
} // namespace mutations
} // namespace apps
} // namespace zpt
