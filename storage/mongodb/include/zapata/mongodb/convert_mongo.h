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

#pragma once

#include <zapata/json/JSONClass.h>
#undef INVALID_SOCKET
#include <mongo/bson/bsonelement.h>
#include <mongo/bson/bsonobjbuilder.h>
#include <mongo/client/dbclient.h>
#define INVALID_SOCKET -1
#include <stddef.h>
#include <string>

namespace mongo {
class ScopedDbConnection;
}

namespace zpt {
namespace mongodb {
void frommongo(mongo::BSONObj& _in, zpt::JSONObj& _out);
void frommongo(mongo::BSONElement& _in, zpt::JSONArr& _out);

void tomongo(zpt::JSONObj& _in, mongo::BSONObjBuilder& _out);
void tomongo(zpt::JSONArr& _in, mongo::BSONArrayBuilder& _out);
void tosetcommand(zpt::JSONObj& _in, mongo::BSONObjBuilder& _out, string _prefix = "");
void tosetcommand(zpt::JSONArr& _in, mongo::BSONObjBuilder& _out, string _prefix);

void get_query(zpt::json _in, mongo::BSONObjBuilder& _queryr);
auto get_fields(zpt::json _opts) -> zpt::json;

float valid_mongo_version();
} // namespace mongodb
} // namespace zpt
