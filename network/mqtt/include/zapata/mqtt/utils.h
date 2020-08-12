/*
Copyright (c) 2017, Muzzley
*/

/**
 * This example uses the mosquitto C library (http://mosquitto.org).
 * In Ubuntu based systems is installable by executing:
 * $ sudo apt-get install libmosquitto0 libmosquitto0-dev
 *
 * Compile with '-lmosquitto'.
 */

#include <mosquitto.h>
#include <string>
#include <zapata/json.h>

namespace zpt {
namespace mqtt {
namespace utils {
auto
check_err(int _return, int _errno, const std::string& _connection, zpt::LogLevel _log_level) -> int;
}
} // namespace mqtt
} // namespace zpt
