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

#include <zapata/text/convert.h>

#include <unistd.h>
#include <iomanip>

auto
zpt::base64::r_encode(std::string const& _in) -> std::string {
    std::string _out(_in.data());
    zpt::base64::encode(_out);
    return _out;
}

void
zpt::base64::encode(std::string& _out) {
    std::istringstream in;
    char buff1[3];
    char buff2[4];
    uint8_t i = 0, j;

    in.str(_out);
    _out.assign("");

    while (in.readsome(&buff1[i++], 1))
        if (i == 3) {
            _out.push_back(encodeCharacterTable[(buff1[0] & 0xfc) >> 2]);
            _out.push_back(
              encodeCharacterTable[((buff1[0] & 0x03) << 4) + ((buff1[1] & 0xf0) >> 4)]);
            _out.push_back(
              encodeCharacterTable[((buff1[1] & 0x0f) << 2) + ((buff1[2] & 0xc0) >> 6)]);
            _out.push_back(encodeCharacterTable[buff1[2] & 0x3f]);
            i = 0;
        }

    if (--i) {
        for (j = i; j < 3; j++) buff1[j] = '\0';

        buff2[0] = (buff1[0] & 0xfc) >> 2;
        buff2[1] = ((buff1[0] & 0x03) << 4) + ((buff1[1] & 0xf0) >> 4);
        buff2[2] = ((buff1[1] & 0x0f) << 2) + ((buff1[2] & 0xc0) >> 6);
        buff2[3] = buff1[2] & 0x3f;

        for (j = 0; j < (i + 1); j++) _out.push_back(encodeCharacterTable[(uint8_t)buff2[j]]);

        while (i++ < 3) _out.push_back('=');
    }
}

auto
zpt::base64::r_decode(std::string const& _in) -> std::string {
    std::string _out(_in.data());
    zpt::base64::decode(_out);
    return _out;
}

void
zpt::base64::decode(std::string& _out) {
    std::istringstream in;
    std::ostringstream out;
    char buff1[4];
    char buff2[4];
    uint8_t i = 0, j;

    in.str(_out);
    _out.assign("");

    while (in.readsome(&buff2[i], 1) && buff2[i] != '=') {
        if (++i == 4) {
            for (i = 0; i != 4; i++) buff2[i] = decodeCharacterTable[(uint8_t)buff2[i]];

            _out.push_back((char)((buff2[0] << 2) + ((buff2[1] & 0x30) >> 4)));
            _out.push_back((char)(((buff2[1] & 0xf) << 4) + ((buff2[2] & 0x3c) >> 2)));
            _out.push_back((char)(((buff2[2] & 0x3) << 6) + buff2[3]));

            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++) buff2[j] = '\0';
        for (j = 0; j < 4; j++) buff2[j] = decodeCharacterTable[(uint8_t)buff2[j]];

        buff1[0] = (buff2[0] << 2) + ((buff2[1] & 0x30) >> 4);
        buff1[1] = ((buff2[1] & 0xf) << 4) + ((buff2[2] & 0x3c) >> 2);
        buff1[2] = ((buff2[2] & 0x3) << 6) + buff2[3];

        for (j = 0; j < (i - 1); j++) _out.push_back((char)buff1[j]);
    }
}

void
zpt::base64::encode(std::istream& _in, std::ostream& _out) {
    char buff1[3];
    char buff2[4];
    uint8_t i = 0, j;

    while (_in.readsome(&buff1[i++], 1))
        if (i == 3) {
            _out << encodeCharacterTable[(buff1[0] & 0xfc) >> 2];
            _out << encodeCharacterTable[((buff1[0] & 0x03) << 4) + ((buff1[1] & 0xf0) >> 4)];
            _out << encodeCharacterTable[((buff1[1] & 0x0f) << 2) + ((buff1[2] & 0xc0) >> 6)];
            _out << encodeCharacterTable[buff1[2] & 0x3f];
            i = 0;
        }

    if (--i) {
        for (j = i; j < 3; j++) buff1[j] = '\0';

        buff2[0] = (buff1[0] & 0xfc) >> 2;
        buff2[1] = ((buff1[0] & 0x03) << 4) + ((buff1[1] & 0xf0) >> 4);
        buff2[2] = ((buff1[1] & 0x0f) << 2) + ((buff1[2] & 0xc0) >> 6);
        buff2[3] = buff1[2] & 0x3f;

        for (j = 0; j < (i + 1); j++) _out << encodeCharacterTable[(uint8_t)buff2[j]];

        while (i++ < 3) _out << '=';
    }
}

void
zpt::base64::decode(std::istream& _in, std::ostream& _out) {
    char buff1[4];
    char buff2[4];
    uint8_t i = 0, j;

    while (_in.readsome(&buff2[i], 1) && buff2[i] != '=') {
        if (++i == 4) {
            for (i = 0; i != 4; i++) buff2[i] = decodeCharacterTable[(uint8_t)buff2[i]];

            _out << (char)((buff2[0] << 2) + ((buff2[1] & 0x30) >> 4));
            _out << (char)(((buff2[1] & 0xf) << 4) + ((buff2[2] & 0x3c) >> 2));
            _out << (char)(((buff2[2] & 0x3) << 6) + buff2[3]);

            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++) buff2[j] = '\0';
        for (j = 0; j < 4; j++) buff2[j] = decodeCharacterTable[(uint8_t)buff2[j]];

        buff1[0] = (buff2[0] << 2) + ((buff2[1] & 0x30) >> 4);
        buff1[1] = ((buff2[1] & 0xf) << 4) + ((buff2[2] & 0x3c) >> 2);
        buff1[2] = ((buff2[2] & 0x3) << 6) + buff2[3];

        for (j = 0; j < (i - 1); j++) _out << (char)buff1[j];
    }
}

auto
zpt::base64::r_url_encode(std::string const& _in) -> std::string {
    std::string _out(_in.data());
    zpt::base64::url_encode(_out);
    return _out;
}

void
zpt::base64::url_encode(std::string& _out) {
    std::istringstream in;
    std::ostringstream out;
    char buff1[4];
    char buff2[4];
    uint8_t i = 0, j;

    in.str(_out);

    while (in.readsome(&buff1[i++], 1))
        if (i == 3) {
            out << encodeCharacterTableUrl[(buff1[0] & 0xfc) >> 2];
            out << encodeCharacterTableUrl[((buff1[0] & 0x03) << 4) + ((buff1[1] & 0xf0) >> 4)];
            out << encodeCharacterTableUrl[((buff1[1] & 0x0f) << 2) + ((buff1[2] & 0xc0) >> 6)];
            out << encodeCharacterTableUrl[buff1[2] & 0x3f];
            i = 0;
        }

    if (--i) {
        for (j = i; j < 3; j++) buff1[j] = '\0';

        buff2[0] = (buff1[0] & 0xfc) >> 2;
        buff2[1] = ((buff1[0] & 0x03) << 4) + ((buff1[1] & 0xf0) >> 4);
        buff2[2] = ((buff1[1] & 0x0f) << 2) + ((buff1[2] & 0xc0) >> 6);
        buff2[3] = buff1[2] & 0x3f;

        for (j = 0; j < (i + 1); j++) out << encodeCharacterTableUrl[(uint8_t)buff2[j]];

        //		while (i++ < 3)
        //			out << '=';
    }
    out << std::flush;
    _out.assign(out.str());
}

auto
zpt::base64::r_url_decode(std::string const& _in) -> std::string {
    std::string _out(_in.data());
    zpt::base64::url_decode(_out);
    return _out;
}

void
zpt::base64::url_decode(std::string& _out) {
    std::istringstream in;
    std::ostringstream out;
    char buff1[4];
    char buff2[4];
    uint8_t i = 0, j;

    while (in.readsome(&buff2[i], 1) && buff2[i] != '=') {
        if (++i == 4) {
            for (i = 0; i != 4; i++) buff2[i] = decodeCharacterTableUrl[(uint8_t)buff2[i]];

            out << (char)((buff2[0] << 2) + ((buff2[1] & 0x30) >> 4));
            out << (char)(((buff2[1] & 0xf) << 4) + ((buff2[2] & 0x3c) >> 2));
            out << (char)(((buff2[2] & 0x3) << 6) + buff2[3]);

            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++) buff2[j] = '\0';
        for (j = 0; j < 4; j++) buff2[j] = decodeCharacterTableUrl[(uint8_t)buff2[j]];

        buff1[0] = (buff2[0] << 2) + ((buff2[1] & 0x30) >> 4);
        buff1[1] = ((buff2[1] & 0xf) << 4) + ((buff2[2] & 0x3c) >> 2);
        buff1[2] = ((buff2[2] & 0x3) << 6) + buff2[3];

        for (j = 0; j < (i - 1); j++) out << (char)buff1[j];
    }
    out << std::flush;
    _out.assign(out.str());
}
