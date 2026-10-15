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
 * @brief Demonstrates the secure key store: create, get, list, rotate, remove.
 *
 * The store persists to a SQLite file (default /tmp/zapata-keystore/keystore) and
 * keeps every secret encrypted. Run it to see the round-trip in action.
 */

#include <iostream>
#include <zapata/config/config.h>
#include <zapata/keystore.h>

int main() {
    zpt::GLOBAL_CONFIG()["keystore"]["path"] = "/tmp/keystore";
    // Root of trust. In production, source this from the environment
    // (ZAPATA_KEYSTORE_PASSPHRASE) or configuration -- never from source.
    auto& _store = zpt::KEY_STORE("demo-passphrase-change-me");

    // Create secrets (optionally with metadata).
    _store.create(
      "api/github", "ghp_secret_token_123", zpt::json{ "scope", "repo", "owner", "zapata" });
    _store.create("db/main", "hunter2");
    std::cout << "created 2 keys\n";

    // Retrieve.
    std::cout << "github token = " << _store.get("api/github") << "\n";

    // List (metadata is shown, secret values never are).
    std::cout << "keys = " << _store.list().stringify() << "\n";

    // Rotate (replaces the value, bumps the version).
    _store.rotate("api/github", "ghp_rotated_token_456");
    std::cout << "rotated github token = " << _store.get("api/github") << "\n";

    // Presence + removal.
    std::cout << "has(db/main) = " << std::boolalpha << _store.has("db/main") << "\n";
    _store.remove("db/main");
    std::cout << "has(db/main) after remove = " << _store.has("db/main") << "\n";

    return 0;
}
