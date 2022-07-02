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

#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <locale>
#include <sstream>
#include <stdio.h>
#include <string.h>

namespace zpt {

void
ltrim(std::string& _in_out);
void
rtrim(std::string& _in_out);
void
trim(std::string& _in_out);
auto
replace(std::string& str, std::string find, std::string replace) -> void;

void
normalize_path(std::string& _in_out, bool _with_trailing);

void
prettify_header_name(std::string& name);

std::string
r_ltrim(std::string const& _in_out);
std::string
r_rtrim(std::string const& _in_out);
std::string
r_trim(std::string const& _in_out);
std::string
r_replace(std::string str, std::string find, std::string replace);

std::string
r_normalize_path(std::string const& _in_out, bool _with_trailing);

std::string
r_prettify_header_name(std::string name);

} // namespace zpt
