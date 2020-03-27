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

#include <zapata/pipeline/pipeline.h>

auto
zpt::pipeline::to_pattern(std::string const& _path) -> zpt::json {
    zpt::regex _regex{ zpt::r_replace(zpt::r_replace(_path, "{", ""), "}", "") };
    try {
        zpt::json _splitted = zpt::uri::to_regex(zpt::uri::parse(_path, zpt::JSArray));
        return { "splitted", _splitted, "regex", _regex };
    }
    catch (zpt::SyntaxErrorException const& _e) {
        return { "splitted", { zpt::array, _regex }, "regex", _regex };
    }
}

auto
zpt::pipeline::to_path(std::string const& _path) -> zpt::json {
    zpt::json _uri = zpt::uri::parse(_path, zpt::JSArray);
    return { "splitted", _uri, "raw", _path };
}
