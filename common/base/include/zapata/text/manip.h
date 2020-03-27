/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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
cipher(std::string const& _in, std::string const& _key, std::string& _out);
void
decipher(std::string const& _in, std::string const& _key, std::string& _out);
void
encrypt(std::string& _out, std::string const& _in, std::string const& _key);
void
decrypt(std::string& _out, std::string const& _in, std::string const& _key);

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
r_cipher(std::string const& _in, std::string const& _key);
std::string
r_decipher(std::string const& _in, std::string const& _key);
std::string
r_encrypt(std::string const& _in, std::string const& _key);
std::string
r_decrypt(std::string const& _in, std::string const& _key);

std::string
r_prettify_header_name(std::string name);

namespace crypto {

namespace gcm {

namespace aes {

void
encrypt(std::string& _out,
        const std::string& _in,
        const std::string& _key,
        const std::string& _iv,
        int _tag_size);
void
decrypt(std::string& _out,
        const std::string& _in,
        const std::string& _key,
        const std::string& _iv,
        int _tag_size);

auto
r_encrypt(const std::string& _in, const std::string& _key, const std::string& _iv, int _tag_size)
  -> std::string;
auto
r_decrypt(const std::string& _in, const std::string& _key, const std::string& _iv, int _tag_size)
  -> std::string;
} // namespace aes
} // namespace gcm
} // namespace crypto
} // namespace zpt
