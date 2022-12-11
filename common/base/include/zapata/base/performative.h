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

#include <string>

namespace zpt {
using performative = unsigned short;
using status = unsigned short;

inline const zpt::performative Get = 0;
inline const zpt::performative Put = 1;
inline const zpt::performative Post = 2;
inline const zpt::performative Delete = 3;
inline const zpt::performative Head = 4;
inline const zpt::performative Options = 5;
inline const zpt::performative Patch = 6;
inline const zpt::performative Reply = 7;
inline const zpt::performative Msearch = 8;
inline const zpt::performative Notify = 9;
inline const zpt::performative Trace = 10;
inline const zpt::performative Connect = 11;
inline const zpt::performative Performative_end = 12;

namespace ontology {
auto to_str(zpt::performative _performative) -> std::string;
auto from_str(std::string _performative) -> zpt::performative;
} // namespace ontology

} // namespace zpt
