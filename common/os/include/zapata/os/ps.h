/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * @file ps.h
 * @brief Process listing for the local system.
 *
 * Enumerates the running processes exposed by the Linux `/proc` pseudo-filesystem,
 * optionally filtered by owning user and/or group.
 *
 * @par Example
 * @code
 * #include <zapata/os.h>
 * #include <unistd.h>
 *
 * // Every readable process on the system
 * zpt::json all = zpt::ps();
 *
 * // Only the processes owned by the current uid and gid
 * zpt::json mine = zpt::ps(getuid(), getgid());
 *
 * // Only the processes owned by a given group, any user
 * zpt::json group = zpt::ps(static_cast<uid_t>(-1), 1000);
 * @endcode
 *
 * @see zpt::ps
 */

#pragma once

#include <sys/types.h>
#include <zapata/json.h>

namespace zpt {

/**
 * @brief Lists running processes, optionally filtered by owner uid and/or gid.
 *
 * Scans `/proc/<pid>/status` for every process. A process is selected when its
 * real UID equals `_uid` and its real GID equals `_gid`; each filter is skipped
 * when its argument is the `static_cast<...>(-1)` sentinel (an id no process can
 * own), which is the default. With both arguments left at their defaults, every
 * readable process is returned. Processes that exit while the scan is in progress
 * are skipped — their disappearance is an expected race on `/proc`, not an error.
 *
 * @param _uid Real user id to filter on, or `static_cast<uid_t>(-1)` for any user.
 * @param _gid Real group id to filter on, or `static_cast<gid_t>(-1)` for any group.
 * @return A `zpt::json` array; each element is an object describing one process:
 *         `pid`, `ppid`, `name`, `state`, `uid`, `euid`, `gid`, `egid`, `cmdline`.
 *         The array is empty when no process matches.
 * @throws std::filesystem::filesystem_error If `/proc` cannot be opened for iteration.
 */
auto ps(uid_t _uid = static_cast<uid_t>(-1), gid_t _gid = static_cast<gid_t>(-1)) -> zpt::json;

} // namespace zpt
